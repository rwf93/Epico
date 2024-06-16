#pragma once

class VulkanImage {
    VkImage &get_image() { return image; }
    VkImageView &get_view() { return view; }
private:
    VkImage image;
    VkImageView view;

    VmaAllocation allocation;
    VkExtent3D extent;

    VkFormat format;
};