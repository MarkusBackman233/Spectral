#include "AssetBrowser.h"
#include <src/IMGUI/imgui.h>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <string>
#include <IOManager.h>
#include <StringUtils.h>
#include <ResourceManager.h>
#include <Texture.h>
namespace fs = std::filesystem;

void AssetBrowser::UpdateLayoutSizes(float avail_width)
{
    // Layout: when not stretching: allow extending into right-most spacing.
    LayoutItemSpacing = (float)IconSpacing;
    if (StretchSpacing == false)
        avail_width += floorf(LayoutItemSpacing * 0.5f);

    // Layout: calculate number of icon per line and number of lines
    LayoutItemSize = ImVec2(floorf(IconSize), floorf(IconSize));
    LayoutColumnCount = std::max((int)(avail_width / (LayoutItemSize.x + LayoutItemSpacing)), 1);
    LayoutLineCount = (Items.size() + LayoutColumnCount - 1) / LayoutColumnCount;

    // Layout: when stretching: allocate remaining space to more spacing. Round before division, so item_spacing may be non-integer.
    if (StretchSpacing && LayoutColumnCount > 1)
        LayoutItemSpacing = floorf(avail_width - LayoutItemSize.x * LayoutColumnCount) / LayoutColumnCount;

    LayoutItemStep = ImVec2(LayoutItemSize.x + LayoutItemSpacing, LayoutItemSize.y + LayoutItemSpacing);
    LayoutSelectableSpacing = std::max(floorf(LayoutItemSpacing) - IconHitSpacing, 0.0f);
    LayoutOuterPadding = floorf(LayoutItemSpacing * 0.5f);
}




AssetBrowser::AssetBrowser()
{


}

void AssetBrowser::Update()
{


    if (!ImGui::Begin("Asset Browser", nullptr, ImGuiWindowFlags_MenuBar))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("My2ColTable", 2, ImGuiTableFlags_Resizable )) {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        FileExplorer();
        ImGui::TableSetColumnIndex(1);
        Assets();


        ImGui::EndTable();
    }

    ImGui::End();
}



void AssetBrowser::FileExplorer()
{
    FileExplorer::ShowSubFolders(IOManager::ProjectDirectory, std::bind(&AssetBrowser::OpenFolder, this, std::placeholders::_1));
}

void FileExplorer::ShowSubFolders(const std::filesystem::path& path, const std::function<void(const std::filesystem::path&)>& onOpenFolder)
{

    static fs::path selectedFolder;


    for (const auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_directory())
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

            fs::path entryPath = entry.path();

            if (selectedFolder == entryPath)
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            bool open = ImGui::TreeNodeEx(entryPath.filename().string().c_str(), flags);

            if (ImGui::IsItemClicked()) 
            {
                selectedFolder = entryPath;
                onOpenFolder(entryPath);
            }

            if (open) 
            {
                FileExplorer::ShowSubFolders(entryPath, onOpenFolder);
                ImGui::TreePop();
            }
        }
    }
}

void AssetBrowser::OpenFolder(const std::filesystem::path& folder)
{
    Items.clear();
    ImGuiIdToIndex.clear();
    for (const auto& entry : fs::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            auto fileItem = FileItem(entry.path());

            ImGuiIdToIndex.emplace(fileItem.m_ID, static_cast<unsigned int>(Items.size()));
            Items.push_back(fileItem);

        }
    }
}


void AssetBrowser::Assets()
{

    // Menu bar
    if (ImGui::BeginMenuBar())
    {

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Delete", "Del", false, Selection.Size > 0))
                RequestDelete = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options"))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * 10);

            ImGui::SeparatorText("Contents");
            ImGui::Checkbox("Show Type Overlay", &ShowTypeOverlay);

            ImGui::SeparatorText("Selection Behavior");
            ImGui::Checkbox("Allow dragging unselected item", &AllowDragUnselected);
            ImGui::Checkbox("Allow box-selection", &AllowBoxSelect);

            ImGui::SeparatorText("Layout");
            ImGui::SliderFloat("Icon Size", &IconSize, 16.0f, 128.0f, "%.0f");
            ImGui::SliderInt("Icon Spacing", &IconSpacing, 0, 32);
            ImGui::SliderInt("Icon Hit Spacing", &IconHitSpacing, 0, 32);
            ImGui::Checkbox("Stretch Spacing", &StretchSpacing);
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }


    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowContentSize(ImVec2(0.0f, LayoutOuterPadding + LayoutLineCount * (LayoutItemSize.y + LayoutItemSpacing)));
    if (ImGui::BeginChild("Assets", ImVec2(0.0f, -ImGui::GetTextLineHeightWithSpacing()), ImGuiChildFlags_None, ImGuiWindowFlags_NoMove))
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const float avail_width = ImGui::GetContentRegionAvail().x;
        UpdateLayoutSizes(avail_width);

        // Calculate and store start position.
        ImVec2 start_pos = ImGui::GetCursorScreenPos();
        start_pos = ImVec2(start_pos.x + LayoutOuterPadding, start_pos.y + LayoutOuterPadding);
        ImGui::SetCursorScreenPos(start_pos);

        // Multi-select
        ImGuiMultiSelectFlags ms_flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid;

        // - Enable box-select (in 2D mode, so that changing box-select rectangle X1/X2 boundaries will affect clipped items)
        if (AllowBoxSelect)
            ms_flags |= ImGuiMultiSelectFlags_BoxSelect2d;

        // - This feature allows dragging an unselected item without selecting it (rarely used)
        if (AllowDragUnselected)
            ms_flags |= ImGuiMultiSelectFlags_SelectOnClickRelease;

        // - Enable keyboard wrapping on X axis
        // (FIXME-MULTISELECT: We haven't designed/exposed a general nav wrapping api yet, so this flag is provided as a courtesy to avoid doing:
        //    ImGui::NavMoveRequestTryWrapping(ImGui::GetCurrentWindow(), ImGuiNavMoveFlags_WrapX);
        // When we finish implementing a more general API for this, we will obsolete this flag in favor of the new system)
        ms_flags |= ImGuiMultiSelectFlags_NavWrapX;

        ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(ms_flags, Selection.Size, Items.size());

        // Use custom selection adapter: store ID in selection (recommended)
        Selection.UserData = this;
        Selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* self_, int idx) { AssetBrowser* self = (AssetBrowser*)self_->UserData; return self->Items[idx].m_ID; };
        Selection.ApplyRequests(ms_io);

        const bool want_delete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (Selection.Size > 0)) || RequestDelete;
        const int item_curr_idx_to_focus = want_delete ? Selection.ApplyDeletionPreLoop(ms_io, Items.size()) : -1;
        RequestDelete = false;

        // Push LayoutSelectableSpacing (which is LayoutItemSpacing minus hit-spacing, if we decide to have hit gaps between items)
        // Altering style ItemSpacing may seem unnecessary as we position every items using SetCursorScreenPos()...
        // But it is necessary for two reasons:
        // - Selectables uses it by default to visually fill the space between two items.
        // - The vertical spacing would be measured by Clipper to calculate line height if we didn't provide it explicitly (here we do).
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(LayoutSelectableSpacing, LayoutSelectableSpacing));

        // Rendering parameters
        const ImU32 icon_type_overlay_colors[3] = { 0, IM_COL32(200, 70, 70, 255), IM_COL32(70, 170, 70, 255) };
        const ImU32 icon_bg_color = ImGui::GetColorU32(IM_COL32(35, 35, 35, 220));
        const ImVec2 icon_type_overlay_size = ImVec2(4.0f, 4.0f);
        const bool display_label = (LayoutItemSize.x >= ImGui::CalcTextSize("999").x);

        const int column_count = LayoutColumnCount;
        ImGuiListClipper clipper;
        clipper.Begin(LayoutLineCount, LayoutItemStep.y);
        if (item_curr_idx_to_focus != -1)
            clipper.IncludeItemByIndex(item_curr_idx_to_focus / column_count); // Ensure focused item line is not clipped.
        if (ms_io->RangeSrcItem != -1)
            clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem / column_count); // Ensure RangeSrc item line is not clipped.
        while (clipper.Step())
        {
            for (int line_idx = clipper.DisplayStart; line_idx < clipper.DisplayEnd; line_idx++)
            {
                const int item_min_idx_for_current_line = line_idx * column_count;
                const int item_max_idx_for_current_line = std::min((line_idx + 1) * column_count, static_cast<int>(Items.size()));
                for (int item_idx = item_min_idx_for_current_line; item_idx < item_max_idx_for_current_line; ++item_idx)
                {

                    FileItem* item_data = &Items[item_idx];
                    ImGui::PushID((int)item_data->m_ID);

                    // Position item
                    ImVec2 pos = ImVec2(start_pos.x + (item_idx % column_count) * LayoutItemStep.x, start_pos.y + line_idx * LayoutItemStep.y);
                    ImGui::SetCursorScreenPos(pos);

                    ImGui::SetNextItemSelectionUserData(item_idx);
                    bool item_is_selected = Selection.Contains((ImGuiID)item_data->m_ID);
                    bool item_is_visible = ImGui::IsRectVisible(LayoutItemSize);
                    ImGui::Selectable("", item_is_selected, ImGuiSelectableFlags_None, LayoutItemSize);

                    // Update our selection state immediately (without waiting for EndMultiSelect() requests)
                    // because we use this to alter the color of our text/icon.
                    if (ImGui::IsItemToggledSelection())
                        item_is_selected = !item_is_selected;

                    // Focus (for after deletion)
                    if (item_curr_idx_to_focus == item_idx)
                        ImGui::SetKeyboardFocusHere(-1);

                    // Drag and drop
                    if (ImGui::BeginDragDropSource())
                    {
                        // Create payload with full selection OR single unselected item.
                        // (the later is only possible when using ImGuiMultiSelectFlags_SelectOnClickRelease)
                        if (ImGui::GetDragDropPayload() == NULL)
                        {
                            ImVector<ImGuiID> payload_items;
                            void* it = NULL;
                            ImGuiID id = 0;
                            if (!item_is_selected)
                                payload_items.push_back(item_data->m_ID);
                            else
                                while (Selection.GetNextSelectedItem(&it, &id))
                                    payload_items.push_back(id);
                            SelectedItems.clear();
                            for (size_t i = 0; i < payload_items.size(); i++)
                            {
                                SelectedItems.push_back(Items[ImGuiIdToIndex[payload_items[i]]]); // im sorry for this :)))))))))))
                            }

                            ImGui::SetDragDropPayload("ASSETS_BROWSER_ITEMS", SelectedItems.data(), sizeof(FileItem) * SelectedItems.size());

                        }

                        // Display payload content in tooltip, by extracting it from the payload data
                        // (we could read from selection, but it is more correct and reusable to read from payload)
                        const ImGuiPayload* payload = ImGui::GetDragDropPayload();
                        const int payload_count = (int)payload->DataSize / (int)sizeof(ImGuiID);
                        ImGui::Text("%d assets", payload_count);

                        ImGui::EndDragDropSource();
                    }

                    // Render icon (a real app would likely display an image/thumbnail here)
                    // Because we use ImGuiMultiSelectFlags_BoxSelect2d, clipping vertical may occasionally be larger, so we coarse-clip our rendering as well.
                    if (item_is_visible)
                    {
                        ImVec2 box_min(pos.x - 1, pos.y - 1);
                        ImVec2 box_max(box_min.x + LayoutItemSize.x + 2, box_min.y + LayoutItemSize.y + 2); // Dubious
                        draw_list->AddRectFilled(box_min, box_max, icon_bg_color); // Background color

                        if (item_data->m_type == ResourceType::Texture)
                        {
                            auto texture = ResourceManager::GetInstance()->GetResource<Texture>(item_data->m_filename);
                            draw_list->AddImage(texture->GetResourceView().Get(), box_min, box_max);
                        }
                        if (item_data->m_type == ResourceType::Material || item_data->m_type == ResourceType::Model)
                        {
                            draw_list->AddImage(item_data->m_thumbnail->GetSRV(), box_min, box_max);
                        }

                        //if (ShowTypeOverlay && item_data->m_type != 0)
                        //{
                        //    ImU32 type_col = icon_type_overlay_colors[item_data->m_type % IM_ARRAYSIZE(icon_type_overlay_colors)];
                        //    draw_list->AddRectFilled(ImVec2(box_max.x - 2 - icon_type_overlay_size.x, box_min.y + 2), ImVec2(box_max.x - 2, box_min.y + 2 + icon_type_overlay_size.y), type_col);
                        //}
                        if (display_label)
                        {
                            
                            ImU32 label_col = ImGui::GetColorU32(ImGuiCol_Text/*item_is_selected ? ImGuiCol_Text : ImGuiCol_TextDisabled*/);
                            //draw_list->AddText(ImVec2(box_min.x, box_max.y - ImGui::GetFontSize()), label_col, item_data->m_filename.c_str());
                            draw_list->AddText(
                                ImGui::GetFont(), 
                                ImGui::GetFontSize(), 
                                ImVec2(box_min.x, box_max.y - ImGui::GetFontSize()), 
                                label_col, 
                                item_data->m_filename.c_str(), 
                                nullptr,
                                box_max.x - box_min.x
                            );
                        }
                    }

                    ImGui::PopID();
                }
            }
        }
        clipper.End();
        ImGui::PopStyleVar(); // ImGuiStyleVar_ItemSpacing

        // Context menu
        if (ImGui::BeginPopupContextWindow())
        {
            ImGui::Text("Selection: %d items", Selection.Size);
            ImGui::Separator();
            if (ImGui::MenuItem("Delete", "Del", false, Selection.Size > 0))
                RequestDelete = true;
            ImGui::EndPopup();
        }

        ms_io = ImGui::EndMultiSelect();
        Selection.ApplyRequests(ms_io);
        if (want_delete)
            Selection.ApplyDeletionPostLoop(ms_io, Items, item_curr_idx_to_focus);

        // Zooming with CTRL+Wheel
        if (ImGui::IsWindowAppearing())
            ZoomWheelAccum = 0.0f;
        if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f && ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsAnyItemActive() == false)
        {
            ZoomWheelAccum += io.MouseWheel;
            if (fabsf(ZoomWheelAccum) >= 1.0f)
            {
                // Calculate hovered item index from mouse location
                // FIXME: Locking aiming on 'hovered_item_idx' (with a cool-down timer) would ensure zoom keeps on it.
                const float hovered_item_nx = (io.MousePos.x - start_pos.x + LayoutItemSpacing * 0.5f) / LayoutItemStep.x;
                const float hovered_item_ny = (io.MousePos.y - start_pos.y + LayoutItemSpacing * 0.5f) / LayoutItemStep.y;
                const int hovered_item_idx = ((int)hovered_item_ny * LayoutColumnCount) + (int)hovered_item_nx;
                //ImGui::SetTooltip("%f,%f -> item %d", hovered_item_nx, hovered_item_ny, hovered_item_idx); // Move those 4 lines in block above for easy debugging

                // Zoom
                IconSize *= powf(1.1f, (float)(int)ZoomWheelAccum);
                IconSize = std::clamp(IconSize, 16.0f, 128.0f);
                ZoomWheelAccum -= (int)ZoomWheelAccum;
                UpdateLayoutSizes(avail_width);

                // Manipulate scroll to that we will land at the same Y location of currently hovered item.
                // - Calculate next frame position of item under mouse
                // - Set new scroll position to be used in next ImGui::BeginChild() call.
                float hovered_item_rel_pos_y = ((float)(hovered_item_idx / LayoutColumnCount) + fmodf(hovered_item_ny, 1.0f)) * LayoutItemStep.y;
                hovered_item_rel_pos_y += ImGui::GetStyle().WindowPadding.y;
                float mouse_local_y = io.MousePos.y - ImGui::GetWindowPos().y;
                ImGui::SetScrollY(hovered_item_rel_pos_y - mouse_local_y);
            }
        }
    }
    ImGui::EndChild();

    ImGui::Text("Selected: %d/%d items", SelectedItems.size(), Items.size());

    ImGui::SameLine();
    ImGui::Text("    Scale");
    ImGui::SameLine();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0)); // smaller height
    ImGui::PushItemWidth(300.0f);
    ImGui::SliderFloat("##Scale", &IconSize, 32.0f, 128.0f);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();

}

FileItem::FileItem(const std::filesystem::path& filename) :
    m_filename(filename.filename().string()),
    m_ID(ImGui::GetID(filename.filename().string().c_str()))
{

    const std::string extension = filename.extension().string();
    m_type = ResourceType::Num;
    for (size_t i = 0; i < static_cast<size_t>(ResourceType::Num); ++i)
    {
        auto& data = IOManager::IOResources[i];
        for (const auto& ext : data.SupportedExtensions)
        {
            if (StringUtils::StringContainsCaseInsensitive(extension, ext))
            {
                m_type = static_cast<ResourceType>(i);
                break;
            }
        }
        if (m_type == ResourceType::Num && !data.SpectralExtension.empty() && StringUtils::StringContainsCaseInsensitive(extension, data.SpectralExtension))
        {
            m_type = static_cast<ResourceType>(i);
            break;
        }
    }

    if (m_type == ResourceType::Material)
    {
        m_thumbnail = ThumbnailManager::GetThumbnail(ResourceManager::GetInstance()->GetResource<DefaultMaterial>(filename).get());
    }
    else if (m_type == ResourceType::Model)
    {
        m_thumbnail = ThumbnailManager::GetThumbnail(
            ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material").get()
        );
    }
}
