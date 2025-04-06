#ifndef PARTICULAS_CORE_INSTANCE_HPP
#define PARTICULAS_CORE_INSTANCE_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <stdexcept>

struct GLFWwindow;

namespace particulas {

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

class Instance {
public:
    Instance(const std::vector<const char*>& enabledLayers = validationLayers,
             const std::vector<const char*>& additionalExtensions = {});
    ~Instance();

    VkInstance get() const { return instance_; }
    bool validationLayersEnabled() const { return validationLayersEnabled_; }

private:
    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
    bool validationLayersEnabled_ = false;

    void createInstance(const std::vector<const char*>& enabledLayers,
                        const std::vector<const char*>& additionalExtensions);
    bool checkValidationLayerSupport(const std::vector<const char*>& layersToCheck) const;
    std::vector<const char*> getRequiredExtensions() const;
    void setupDebugMessenger();
};

} // namespace particulas

#endif // PARTICULAS_CORE_INSTANCE_HPP