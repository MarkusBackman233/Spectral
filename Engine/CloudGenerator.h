#pragma once
#include <numeric>
#include <vector>
#include <random>
#include <wrl/client.h>
#include <d3d11.h>

class Perlin {
public:
    Perlin() {
        // Initialize permutation vector with reference values
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);

        // Shuffle using a random engine
        std::default_random_engine engine(std::random_device{}());
        std::shuffle(p.begin(), p.end(), engine);

        // Duplicate the permutation vector
        p.insert(p.end(), p.begin(), p.end());
    }

    // 2D Perlin noise
    double noise(double x, double y) const {
        // Find unit grid cell containing point
        int X = int(floor(x)) & 255;
        int Y = int(floor(y)) & 255;

        // Get relative x, y inside cell
        x -= floor(x);
        y -= floor(y);

        // Compute fade curves
        double u = fade(x);
        double v = fade(y);

        // Hash coordinates of the 4 corners
        int aa = p[p[X] + Y];
        int ab = p[p[X] + Y + 1];
        int ba = p[p[X + 1] + Y];
        int bb = p[p[X + 1] + Y + 1];

        // Add blended results from corners
        return lerp(v,
            lerp(u, grad(aa, x, y), grad(ba, x - 1, y)),
            lerp(u, grad(ab, x, y - 1), grad(bb, x - 1, y - 1))
        );
    }

private:
    std::vector<int> p;

    static double fade(double t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    static double lerp(double t, double a, double b) {
        return a + t * (b - a);
    }

    static double grad(int hash, double x, double y) {
        int h = hash & 7;      // Convert low 3 bits of hash code
        double u = h < 4 ? x : y;
        double v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0 * v : 2.0 * v);
    }
};

class Mesh;
class SkyboxManager;
class DeviceResources;

class CloudGenerator
{
public:
    CloudGenerator();
    void CreateResources(ID3D11Device* device);
    void Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources, const SkyboxManager& skyboxManager);

private:
    Perlin m_perlin;
    std::shared_ptr<Mesh> m_cloudMesh;

    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_cloudPixelShader;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_cloudVertexShader;

    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_pInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>	   m_pPixelConstantBufferData;
    Microsoft::WRL::ComPtr<ID3D11Buffer>	   m_pVertexConstantBufferData;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_resource;

};

