#pragma once

// ==============================
// Code Template Macros
// ==============================
#pragma region

// Delete copy and move operations for a Type.
#define nbl_DISABLE_COPY(TYPE)              \
    TYPE(const TYPE&) = delete;             \
    TYPE& operator=(const TYPE&) = delete;  \
    TYPE(const TYPE&&) = delete;            \
    TYPE& operator=(const TYPE&&) = delete;

// Define a constructor with a "CreateInfo" type struct + static uptr creation method.
#define nbl_CI_CTOR(TYPE, CREATE_INFO_TYPE)                                                 \
    explicit TYPE(const CREATE_INFO_TYPE& createInfo);                                      \
    inline static std::unique_ptr<TYPE> create##TYPE(const CREATE_INFO_TYPE& createInfo) {  \
        return std::make_unique<TYPE>(createInfo);                                          \
    }

#pragma endregion