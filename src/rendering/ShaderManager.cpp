#include "ShaderManager.h"
#include <iostream>

bool ShaderManager::loadAll(const std::string& shaderDir) {
    if (!sf::Shader::isAvailable()) {
        std::cerr << "[ShaderManager] Shaders not supported on this hardware\n";
        return false;
    }
    m_shadersAvailable = true;

    struct { ShaderType type; const char* file; } entries[] = {
        { ShaderType::Scanlines, "scanlines.frag" },
        { ShaderType::Vignette,  "vignette.frag"  },
    };

    bool allOk = true;
    for (const auto& e : entries) {
        size_t idx = static_cast<size_t>(e.type);
        std::string path = shaderDir + "/" + e.file;
        if (m_shaders[idx].loadFromFile(path, sf::Shader::Fragment)) {
            m_loaded[idx] = true;
        } else {
            std::cerr << "[ShaderManager] Failed to load: " << path << "\n";
            allOk = false;
        }
    }
    return allOk;
}

sf::Shader* ShaderManager::get(ShaderType type) {
    size_t idx = static_cast<size_t>(type);
    if (!m_shadersAvailable || !m_loaded[idx]) return nullptr;
    return &m_shaders[idx];
}
