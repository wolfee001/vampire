#pragma once

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include <map>
#include <unordered_map>

#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <type_traits>

#include <fmt/format.h>
#include <imgui.h>

#define ___AddGuiElementFirstArgName_(V, ...) #V
#define ___AddGuiElementFirstArgName(args) ___AddGuiElementFirstArgName_ args
// clang-format off
#ifdef _WIN32
#define AddGuiElement(...) ___AddGuiElement(__VA_ARGS__)
#else
#define AddGuiElement(...) \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wgnu-zero-variadic-macro-arguments\"") \
    ___AddGuiElement(__VA_ARGS__) \
    _Pragma("clang diagnostic pop")
#endif
// clang-format on
#define ___AddGuiElement(...) ___PropertyTree::___AddElement(___AddGuiElementFirstArgName((__VA_ARGS__)), __VA_ARGS__)
#define BindGetter(function, object) std::bind(&function, object)
#define BindSetter(function, object) std::bind(&function, object, std::placeholders::_1)

class GuiElement {
    friend class ___PropertyTree;

public:
    virtual ~GuiElement() = default;

protected:
    virtual std::string ToString() const
    {
        return "";
    }
    virtual void DrawGui() = 0;
};

class ___PropertyTree {
public:
    inline static void ___AddElement(const char* name, const GuiElement& value, bool createNode = true)
    {
        ___AddElement(name, const_cast<GuiElement&>(value), createNode);
    }

    inline static void ___AddElement(const char* name, GuiElement& value, bool createNode = true)
    {
        if (createNode) {
            if (ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Framed)) {
                value.DrawGui();
                ImGui::TreePop();
            }
        } else {
            value.DrawGui();
        }
    }

    template <typename T> inline static void ___AddElement(const char* name, const T& value)
    {
        ___AddElement(name, const_cast<T&>(value));
    }

    template <typename T> inline static void ___AddElement(const char* name, T& value)
    {
        if (std::is_base_of<GuiElement, T>::value) {
            return ___AddElement(name, reinterpret_cast<const GuiElement&>(value));
        }

        auto it = mTypeMap.find(typeid(T).name());
        if (it != mTypeMap.end()) {
            AddBaseElement(name, &value, it->second);
        } else {
            AddUnknownElement(name);
        }
    }

    inline static void ___AddElement(const char* name, std::string& value)
    {
        ImGui::PushItemWidth(150);
        ImGui::InputText(fmt::format("{} ({})", name, "string").c_str(), const_cast<char*>(value.c_str()), value.capacity() + 1,
            ImGuiInputTextFlags_CallbackResize, InputTextCallback, reinterpret_cast<void*>(&value));
    }

    template <typename T, typename SetterCallback> inline static void ___AddElement(const char* name, T& value, SetterCallback setter)
    {
        auto it = mTypeMap.find(typeid(T).name());
        if (it == mTypeMap.end()) {
            throw std::runtime_error("Unkown base-like type with setter!");
        }
        AddBaseElement(name, &value, it->second, setter);
    }

    template <typename GetterCallback, typename SetterCallback>
    inline static void ___AddElement(const char*, const char* name, GetterCallback getter, SetterCallback setter)
    {
        auto value = getter();
        ___AddElement(name, value, setter);
    }

    template <typename T, size_t N> inline static void ___AddElement(const char* name, std::array<T, N>& value)
    {
        RollOutSequenceLikeContainer(name, value.begin(), value.end());
    }
    template <typename T> inline static void ___AddElement(const char* name, std::vector<T>& value)
    {
        RollOutSequenceLikeContainer(name, value.begin(), value.end());
    }
    template <typename T> inline static void ___AddElement(const char* name, std::deque<T>& value)
    {
        RollOutSequenceLikeContainer(name, value.begin(), value.end());
    }
    template <typename T> inline static void ___AddElement(const char* name, std::forward_list<T>& value)
    {
        RollOutSequenceLikeContainer(name, value.begin(), value.end());
    }
    template <typename T> inline static void ___AddElement(const char* name, std::list<T>& value)
    {
        RollOutSequenceLikeContainer(name, value.begin(), value.end());
    }
    template <typename T> inline static void ___AddElement(const char* name, std::set<T>& value)
    {
        RollOutSequenceLikeContainer(name, value.begin(), value.end());
    }
    template <typename T> inline static void ___AddElement(const char* name, std::multiset<T>& value)
    {
        RollOutSequenceLikeContainer(name, value.begin(), value.end());
    }
    template <typename T> inline static void ___AddElement(const char* name, std::unordered_set<T>& value)
    {
        RollOutSequenceLikeContainer(name, value.begin(), value.end());
    }
    template <typename T> inline static void ___AddElement(const char* name, std::unordered_multiset<T>& value)
    {
        RollOutSequenceLikeContainer(name, value.begin(), value.end());
    }
    template <typename T, typename H> inline static void ___AddElement(const char* name, std::unordered_set<T, H>& value)
    {
        RollOutSequenceLikeContainer(name, value.begin(), value.end());
    }
    template <typename T, typename H> inline static void ___AddElement(const char* name, std::unordered_multiset<T, H>& value)
    {
        RollOutSequenceLikeContainer(name, value.begin(), value.end());
    }

    template <typename TK, typename TV> inline static void ___AddElement(const char* name, std::map<TK, TV>& value)
    {
        RollOutMapLikeContainer(name, value.begin(), value.end());
    }
    template <typename TK, typename TV> inline static void ___AddElement(const char* name, std::multimap<TK, TV>& value)
    {
        RollOutMapLikeContainer(name, value.begin(), value.end());
    }
    template <typename TK, typename TV> inline static void ___AddElement(const char* name, std::unordered_map<TK, TV>& value)
    {
        RollOutMapLikeContainer(name, value.begin(), value.end());
    }
    template <typename TK, typename TV> inline static void ___AddElement(const char* name, std::unordered_multimap<TK, TV>& value)
    {
        RollOutMapLikeContainer(name, value.begin(), value.end());
    }
    template <typename TK, typename TV, typename TH> inline static void ___AddElement(const char* name, std::unordered_map<TK, TV, TH>& value)
    {
        RollOutMapLikeContainer(name, value.begin(), value.end());
    }
    template <typename TK, typename TV, typename TH> inline static void ___AddElement(const char* name, std::unordered_multimap<TK, TV, TH>& value)
    {
        RollOutMapLikeContainer(name, value.begin(), value.end());
    }

private:
    struct DataTypeDescriptor {
        std::string name;
        ImGuiDataType_ imGuiType;
        int componentCount;
    };

    static int InputTextCallback(ImGuiInputTextCallbackData* data)
    {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
            std::string* str = static_cast<std::string*>(data->UserData);
            str->resize(static_cast<size_t>(data->BufTextLen));
            data->Buf = const_cast<char*>(str->c_str());
        }
        return 0;
    }

    template <typename T> inline static void AddBaseElement(const char* name, T* value, const DataTypeDescriptor& type)
    {
        ImGui::PushItemWidth(150);
        ImGui::InputScalarN(fmt::format("{} ({})", name, type.name).c_str(), type.imGuiType, static_cast<void*>(value), type.componentCount);
    }

    template <typename T, typename SetterCallback>
    inline static void AddBaseElement(const char* name, T* value, const DataTypeDescriptor& type, SetterCallback setter)
    {
        T dummy = *value;
        ImGui::PushItemWidth(150);
        if (ImGui::InputScalarN(fmt::format("{} ({})", name, type.name).c_str(), type.imGuiType, static_cast<void*>(&dummy), type.componentCount)) {
            setter(dummy);
        }
    }

    inline static void AddUnknownElement(const char* name)
    {
        ImGui::PushItemWidth(150);
        ImGui::Text("%s (unknown type)", name);
    }

    template <typename T> inline static void RollOutSequenceLikeContainer(const char* name, const T& begin, const T& end)
    {
        if (ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Framed)) {
            size_t i = 0;
            for (auto it = begin; it != end; ++it) {
                ___AddElement(fmt::format("[{:d}]", i).c_str(), *it);
                ++i;
            }
            ImGui::TreePop();
        }
    }

    template <typename T> inline static void RollOutMapLikeContainer(const char* name, const T& begin, const T& end)
    {
        if (ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Framed)) {
            size_t i = 0;
            for (auto it = begin; it != end; ++it) {
                std::string keyValue;
                if (std::is_base_of<GuiElement, decltype(it->first)>::value) {
                    keyValue = reinterpret_cast<const GuiElement&>(it->first).ToString();
                } else if constexpr (std::is_fundamental<decltype(it->first)>::value) {
                    keyValue = std::to_string(it->first);
                } else if constexpr (std::is_convertible<decltype(it->first), std::string>::value) {
                    keyValue = static_cast<std::string>(it->first);
                } else if constexpr (std::is_convertible<decltype(it->first), const std::string>::value) {
                    keyValue = static_cast<const std::string>(it->first);
                } else if constexpr (std::is_convertible<decltype(it->first), char*>::value) {
                    keyValue = static_cast<char*>(it->first);
                } else if constexpr (std::is_convertible<decltype(it->first), const char*>::value) {
                    keyValue = static_cast<const char*>(it->first);
                } else {
                    keyValue = typeid(it->first).name();
                }
                ___AddElement(fmt::format("{} [{}] {} ->", keyValue, i, typeid(it->first).name()).c_str(), it->second);

                ++i;
            }
            ImGui::TreePop();
        }
    }

private:
    static const std::unordered_map<std::string, DataTypeDescriptor> mTypeMap;
};
