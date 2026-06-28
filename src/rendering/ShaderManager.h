#pragma once
#include <SFML/Graphics.hpp>
#include <array>

enum class ShaderType : uint8_t {
    Scanlines = 0,
    Vignette  = 1,
    COUNT     = 2
};

// Loads and caches all GLSL shaders at startup. Never compiles at runtime.
class ShaderManager {
public:
    ShaderManager() = default;

    // Call once in Game::init(). Returns false if shaders are unsupported.
    bool loadAll(const std::string& shaderDir);

    sf::Shader* get(ShaderType type);
    bool        shadersAvailable() const { return m_shadersAvailable; }

private:
    static constexpr size_t N = static_cast<size_t>(ShaderType::COUNT);
    sf::Shader  m_shaders[N]; // C-array avoids deleted copy ctor issue
    bool        m_loaded[N]{};
    bool        m_shadersAvailable = false;
};
