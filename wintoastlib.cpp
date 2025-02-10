/*
 * Copyright 2025 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "notification.hpp"

#include <memory>
#include <assert.h>
#include <unordered_map>
#include <array>
#include <functional>

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "user32")

using namespace rainy;

#define DEFAULT_SHELL_LINKS_PATH L"\\Microsoft\\Windows\\Start Menu\\Programs\\"
#define DEFAULT_LINK_FORMAT      L".lnk"

/* rainy的代码引用 */
namespace rainy {
    template <typename Fx>
    class function_pointer;

    /**
     * @brief 此模板允许用户创建一个具有类型安全的函数指针对象
     * @tparam Rx 函数指针期望的返回类型
     * @tparam Args 函数形参列表
     */
    template <typename Rx, typename... Args>
    class function_pointer<Rx(Args...)> {
    public:
        using pointer = Rx(*)(Args...);

        /**
         * @brief 仅创建一个基本的函数指针对象
         */
        constexpr function_pointer() : invoker(nullptr) {
        }

        /**
         * @brief 同默认构造函数
         */
        constexpr function_pointer(std::nullptr_t) : invoker(nullptr) {
        }

        /**
         * @brief 用函数地址初始化此函数对象
         */
        constexpr function_pointer(Rx(*function_address)(Args...)) : invoker(function_address) {
        }

        /**
         * @brief 用可能可以转换的函数地址初始化此函数对象
         * @brief 例如，如果模板形参返回值是int，而函数地址对应的签名为char，则产生转换
         */
        template <typename URx, typename... UArgs,
            std::enable_if_t<std::is_invocable_r_v<Rx, URx(*)(UArgs...), Args...>, int> = 0>
        constexpr function_pointer(URx(*function_address)(UArgs...)) : invoker(function_address) {
        }

        constexpr function_pointer(const function_pointer&) noexcept = default;
        constexpr function_pointer(function_pointer&&) noexcept = default;

        ~function_pointer() = default;

        /**
         * @brief 获取当前函数指针对象保存的invoke地址
         */
        RAINY_NODISCARD constexpr pointer get() const noexcept {
            return invoker;
        }

        /**
         * @brief 获取当前函数指针对象保存的invoke地址
         */
        RAINY_NODISCARD explicit operator pointer() const noexcept {
            return invoker;
        }

        /**
         * @brief 获取当前模板中形参的个数
         */
        RAINY_NODISCARD static constexpr unsigned int arity() noexcept {
            return sizeof...(Args);
        }

        /**
         * @brief 检查当前函数指针对象是否为空
         */
        RAINY_NODISCARD constexpr bool empty() const noexcept {
            return !static_cast<bool>(invoker);
        }

        /**
         * @brief 检查当前函数指针对象是否为空
         */
        constexpr explicit operator bool() const noexcept {
            return static_cast<bool>(invoker);
        }

        /**
         * @brief 调用当前函数指针对象，并传入参数列表
         * @return 如果函数返回值是void，则返回void，否则返回函数返回值
         */
        constexpr Rx invoke(Args... args) const {
            if (empty()) {
                throw std::runtime_error("Current pointer is null!");
            }
            if constexpr (std::is_void_v<Rx>) {
                invoker(args...);
            }
            else {
                return invoker(args...);
            }
        }

        /**
         * @brief 调用当前函数指针对象，并传入参数列表
         * @return 如果函数返回值是void，则返回void，否则返回函数返回值
         */
        constexpr Rx operator()(Args... args) const {
            if (empty()) {
                throw std::runtime_error("Current pointer is null!");
            }
            if constexpr (std::is_void_v<Rx>) {
                invoker(args...);
            }
            else {
                return invoker(args...);
            }
        }

        /**
         * @brief 赋值给当前函数指针对象，并返回引用
         * @return 当前函数指针对象引用
         */
        constexpr function_pointer& operator=(pointer function_address) noexcept {
            return assign(function_address); // NOLINT
        }

        /**
         * @brief 赋值给当前函数指针对象，并返回引用
         * @return 当前函数指针对象引用
         */
        constexpr function_pointer& operator=(const function_pointer& right) noexcept {
            return assign(right); // NOLINT
        }

        /**
         * @brief 将当前函数指针对象置空，并返回引用
         * @return 当前函数指针对象引用
         */
        constexpr function_pointer& operator=(std::nullptr_t) noexcept {
            return assign(nullptr); // NOLINT
        }

        /**
         * @brief 移动赋值给当前函数指针对象，并返回引用
         * @attetion 使用默认实现
         */
        constexpr function_pointer& operator=(function_pointer&&) noexcept = default;

        

        /**
         * @brief 赋值给当前函数指针对象，并返回引用
         * @return 当前函数指针对象引用
         */
        constexpr function_pointer& assign(pointer function_address) noexcept {
            invoker = function_address;
            return *this;
        }

        /**
         * @brief 赋值给当前函数指针对象，并返回引用
         * @return 当前函数指针对象引用
         */
        constexpr function_pointer& assign(const function_pointer& right) noexcept {
            if (this != std::addressof(right)) {
                invoker = right.invoker;
            }
            return *this;
        }

        /**
         * @brief 将当前函数指针对象置空，并返回引用
         * @return 当前函数指针对象引用
         */
        constexpr function_pointer& assign(std::nullptr_t) noexcept {
            invoker = nullptr;
            return *this;
        }

        /**
         * @brief 移动赋值给当前函数指针对象，并返回引用
         * @return 当前函数指针对象引用
         */
        template <typename URx, typename... UArgs,
            std::enable_if_t<
            std::is_invocable_r_v<Rx, URx(*)(UArgs...), Args...>, int> = 0>
        constexpr function_pointer& assign(URx(*function_address)(UArgs...)) noexcept {
            invoker = function_address;
            return *this;
        }

        /**
         * @brief 使用新的函数地址替换当前函数指针对象，并返回旧的函数指针对象
         * @return
         * 旧的函数指针对象
         */
        constexpr function_pointer reset(pointer function_address = nullptr) noexcept {
            pointer old_ptr = invoker;
            invoker = function_address;
            return function_pointer{ old_ptr };
        }

        /**
         * @brief 使用新的函数地址替换当前函数指针对象，并返回旧的函数指针对象
         * @return
         * 旧的函数指针对象
         */
        template <typename URx, typename... UArgs,
            std::enable_if_t<
            std::is_invocable_r_v<Rx, URx(*)(UArgs...), Args...>, int> = 0>
        constexpr function_pointer reset(URx(*function_address)(UArgs...) = nullptr) noexcept {
            pointer old_ptr = invoker;
            invoker = function_address;
            return function_pointer{ old_ptr };
        }

        /**
         * @brief 置空当前函数指针对象
         */
        constexpr void clear() noexcept {
            invoker = nullptr;
        }

        /**
         * @brief 交换当前函数指针对象和另一个函数指针对象
         */
        constexpr void swap(function_pointer& right) noexcept {
            std::swap(invoker, right.invoker);
        }

        /**
         * @brief 交换当前函数指针对象和另一个函数指针对象
         */
        constexpr void swap(function_pointer&& right) noexcept {
            std::swap(invoker, right.invoker);
        }

        /**
         * @brief 将函数指针转换为std::function对象
         * @return std::function对象
         */
        RAINY_NODISCARD std::function<Rx(Args...)> make_function_object() const noexcept {
            return std::function<Rx(Args...)>(invoker);
        }

    private:
        pointer invoker;
    };
 
    /**
     * @brief 辅助模板，用于建立function_pointer别名
     */
    template <typename Rx, typename... Args>
    class function_pointer<Rx(*)(Args...)> : public function_pointer<Rx(Args...)> {};

    /**
     * @brief   创建一个函数指针对象
     * @tparam  Rx 函数指针期望的返回类型
     * @tparam  Args 函数形参列表
     * @param   ptr 函数地址
     * @return  函数指针对象
     */
    template <typename Rx, typename... Args>
    constexpr auto make_function_pointer(Rx(*ptr)(Args...)) noexcept -> function_pointer<Rx(Args...)> {
        return function_pointer<Rx(Args...)>(ptr);
    }
}

namespace rainy {
    enum class load_runtime_fn_errcode {
        ok, // 成功
        proc_not_found, // 无法从导出表找到proc
        invalid_handle, // 句柄无效
        access_denied,  // 拒绝访问
        bad_format, // 格式错误
        load_library_failed,// 加载库失败
        unknown_error   // 未知错误
    };

    template <typename FunctionPointerSignature>
    load_runtime_fn_errcode load_runtime_fn(HMODULE handle, const std::string_view function_name,
        function_pointer<FunctionPointerSignature>& fp) noexcept {
        if (!handle) {
            return load_runtime_fn_errcode::invalid_handle;
        }
        auto address = reinterpret_cast<FunctionPointerSignature>(GetProcAddress(handle, function_name.data()));
        if (!address) {
            switch (GetLastError()) {
            case ERROR_PROC_NOT_FOUND:
                return load_runtime_fn_errcode::proc_not_found;
            case ERROR_INVALID_HANDLE:
                return load_runtime_fn_errcode::invalid_handle;
            case ERROR_ACCESS_DENIED:
                return load_runtime_fn_errcode::access_denied;
            case ERROR_BAD_FORMAT:
                return load_runtime_fn_errcode::bad_format;
            default:
                return load_runtime_fn_errcode::unknown_error;
            }
        }
        fp.reset(address);
        return load_runtime_fn_errcode::ok;
    }

    HRESULT set_current_process_aumi(std::wstring_view app_id) {
        function_pointer<HRESULT(*)(PCWSTR)> fp;
        if (!fp) {
            HMODULE dynamic = LoadLibraryW(L"Shell32.dll");
            auto code = load_runtime_fn(dynamic, "SetCurrentProcessExplicitAppUserModelID", fp);
            if (code != load_runtime_fn_errcode::ok) {
                return E_FAIL;
            }
        }
        return fp(app_id.data());
    }
}

namespace util {
    inline RTL_OSVERSIONINFOW get_real_os_version() {
        static rainy::function_pointer<NTSTATUS(*)(PRTL_OSVERSIONINFOW)> fx_ptr;
        RTL_OSVERSIONINFOW rovi = { 0 };
        HMODULE module = ::GetModuleHandleW(L"ntdll.dll");
        if (module) {
            if (!fx_ptr) {
                auto code = load_runtime_fn(module, "RtlGetVersion", fx_ptr);
                if (code != rainy::load_runtime_fn_errcode::ok) {
                    return rovi;
                }
            }
            rovi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
            if (fx_ptr(&rovi) == ERROR_SUCCESS) {
                return rovi;
            }
        }
        return rovi;
    }

    inline HRESULT get_default_executable_path(WCHAR* path, DWORD n_size = MAX_PATH) {
        DWORD written = ::GetModuleFileNameExW(GetCurrentProcess(), nullptr, path, n_size);
        return (written > 0) ? S_OK : E_FAIL;
    }

    inline HRESULT get_default_shell_links_path(WCHAR* path, DWORD n_size = MAX_PATH) {
        DWORD written = GetEnvironmentVariableW(L"APPDATA", path, n_size);
        HRESULT hr = written > 0 ? S_OK : E_INVALIDARG;
        if (SUCCEEDED(hr)) {
            errno_t result = wcscat_s(path, n_size, DEFAULT_SHELL_LINKS_PATH);
            hr = (result == 0) ? S_OK : E_INVALIDARG;
        }
        return hr;
    }

    inline HRESULT get_default_shell_link_path(std::wstring_view appname, WCHAR* path, DWORD n_size = MAX_PATH) {
        HRESULT hr = get_default_shell_links_path(path, n_size);
        if (SUCCEEDED(hr)) {
            std::wstring app_link(appname.data());
            app_link += DEFAULT_LINK_FORMAT;
            errno_t result = wcscat_s(path, n_size, app_link.c_str());
            hr = (result == 0) ? S_OK : E_INVALIDARG;
        }
        return hr;
    }

	inline PCWSTR as_string(winrt::Windows::Data::Xml::Dom::XmlDocument const& xml_document) {
	    try {
	        winrt::hstring xml = xml_document.GetXml();
	        return xml.c_str();
	    }
	    catch (const winrt::hresult_error&) {
	        return nullptr;
	    }
	}

    inline HRESULT set_node_string_value(
        std::wstring_view string,
        winrt::Windows::Data::Xml::Dom::IXmlNode* node,
        const winrt::Windows::Data::Xml::Dom::XmlDocument& xml) {
        try {
            auto text_node = xml.CreateTextNode(winrt::hstring{ string });
            node->AppendChild(text_node);
            return S_OK;
        }
        catch (const winrt::hresult_error& e) {
            return e.code();
        }
    }

    template<typename FunctorT>
    inline winrt::hresult set_event_handlers(winrt::Windows::UI::Notifications::ToastNotification& notification,
        std::shared_ptr<notification_handler> event_handler,
        INT64 expiration_time,
        winrt::event_token& activated_token,
        winrt::event_token& dismissed_token,
        winrt::event_token& failed_token,
        FunctorT&& mark_as_ready_for_deletion_func) {

        activated_token = notification.Activated([event_handler, mark_as_ready_for_deletion_func](auto&& sender, auto&& args) {
            if (auto activated_args = args.try_as<winrt::Windows::UI::Notifications::ToastActivatedEventArgs>()) {
                winrt::hstring arguments = activated_args.Arguments();
                if (arguments == L"action=reply") {
                    auto user_input = activated_args.UserInput();  // Already have activated_args, no need to check again
                    if (winrt::Windows::Foundation::IPropertyValue reply_value = user_input.Lookup(L"textBox").try_as<winrt::Windows::Foundation::IPropertyValue>()) {
                        auto input = reply_value.GetString();
                        event_handler->activated(input);
                        return;
                    }
                }
                if (!arguments.empty()) {
                    event_handler->activated(std::stoi(arguments.c_str()));
                }
                else {
                    event_handler->activated();
                }
            }
            mark_as_ready_for_deletion_func();
            });

        dismissed_token = notification.Dismissed([event_handler, expiration_time, mark_as_ready_for_deletion_func](auto&& sender, auto&& args) {
            auto reason = args.Reason();
            winrt::clock::time_point expiration_time_point(winrt::clock::from_time_t(expiration_time));
            if (reason == winrt::Windows::UI::Notifications::ToastDismissalReason::UserCanceled && expiration_time &&
                winrt::clock::now() >= expiration_time_point) {
                reason = winrt::Windows::UI::Notifications::ToastDismissalReason::TimedOut;
            }
            event_handler->dismissed(static_cast<notification_handler::dismissal_reason>(reason));
            mark_as_ready_for_deletion_func();
            });

        failed_token = notification.Failed([event_handler, mark_as_ready_for_deletion_func](auto&& sender, auto&& args) {
            event_handler->failed();
            mark_as_ready_for_deletion_func();
            });

        return S_OK;
    }

    inline HRESULT add_attribute(winrt::Windows::Data::Xml::Dom::XmlDocument const& xml, std::wstring_view name, winrt::Windows::Data::Xml::Dom::XmlNamedNodeMap const& attribute_map) {
        try {
            winrt::Windows::Data::Xml::Dom::XmlAttribute src_attribute = xml.CreateAttribute(name); // 创建属性
            attribute_map.SetNamedItem(src_attribute); // 将属性添加到节点属性集合
            return S_OK;
        }
        catch (const winrt::hresult_error& e) {
            return e.code();
        }
    }

    inline HRESULT create_element(winrt::Windows::Data::Xml::Dom::XmlDocument const& xml,
        std::wstring_view root_node,
        std::wstring_view element_name,
        const std::vector<std::wstring>& attribute_names) {
        try {
            // 获取根节点列表
            auto root_list = xml.GetElementsByTagName(root_node);
            if (root_list.Size() > 0) {
                // 获取根节点
                auto root = root_list.GetAt(0);
                // 创建元素
                winrt::Windows::Data::Xml::Dom::XmlElement audio_element = xml.CreateElement(element_name);
                // 将元素添加到根节点
                root.AppendChild(audio_element);
                // 获取该节点的属性集合
                auto attributes = audio_element.Attributes();
                // 添加属性
                for (auto const& attr : attribute_names) {
                    add_attribute(xml, attr, attributes);
                }
                return S_OK;
            }
            return E_FAIL;
        }
        catch (const winrt::hresult_error& e) {
            return e.code();
        }
    }
} // namespace util

notification::notification() : is_initialized_(false), has_winrt_initialized_(false) {}

notification::~notification() {
    clear();
    if (has_winrt_initialized_) {
        CoUninitialize();
    }
}

void notification::set_app_name(std::wstring const& app_name) {
    appname_ = app_name;
}

void notification::set_aumi(std::wstring_view aumi) {
    aumi_ = aumi;
}

void notification::set_shortcut_policy(utility::shortcut_policy shortcut_policy) {
    shortcut_policy_ = shortcut_policy;
}

bool notification::is_supporting_modern_features() {
    constexpr auto MinimumSupportedVersion = 6;
    return util::get_real_os_version().dwMajorVersion > MinimumSupportedVersion;
}

bool notification::is_win10_anniversary_or_higher() {
    return util::get_real_os_version().dwBuildNumber >= 14393;
}

std::wstring notification::make_aumi(std::wstring const& company_name, std::wstring const& product_name,
    std::wstring const& sub_product,
    std::wstring const& version_information) {
    std::wstring aumi = company_name + L"." + product_name;
    if (sub_product.length() > 0) {
        aumi += L"." + sub_product;
        if (version_information.length() > 0) {
            aumi += L"." + version_information;
        }
    }
    return aumi;
}

std::wstring const& notification::strerror(notification_error error) {
    static const std::unordered_map<notification_error, std::wstring> Labels = {
        {notification_error::no_error,                     L"No error. The process was executed correctly"                                  },
        {notification_error::not_initialized,              L"The library has not been initialized"                                          },
        {notification_error::system_not_supported,         L"The OS does not support notification"                                              },
        {notification_error::shell_link_not_created,       L"The library was not able to create a Shell Link for the app"                   },
        {notification_error::invalid_app_user_model_id,    L"The AUMI is not a valid one"                                                   },
        {notification_error::invalid_parameters,           L"Invalid parameters, please double-check the AUMI or App Name"                  },
        {notification_error::not_displayed,                L"The toast was created correctly but notification was not able to display the toast"},
        {notification_error::unknown_error,                L"Unknown error"                                                                 }
    };

    auto const iter = Labels.find(error);
    assert(iter != Labels.end());
    return iter->second;
}

utility::shortcut_result utility::create_shortcut(shortcut_policy policy,std::wstring_view appname, std::wstring_view aumi, bool& winrt_init_flag) {
    if (aumi.empty() || appname.empty()) {
        return utility::shortcut_result::missing_parameters;
    }
    if (!winrt_init_flag) {
        try {
            winrt::init_apartment(winrt::apartment_type::multi_threaded);
        }
        catch (const winrt::hresult_error&) {
            return utility::shortcut_result::com_init_failure;
        }
        winrt_init_flag = true;
    }
    bool was_changed = false;
    HRESULT hr = validate_shelllink(was_changed, appname, aumi);
    if (SUCCEEDED(hr)) {
        return was_changed ? utility::shortcut_result::was_changed : utility::shortcut_result::unchanged;
    }
    hr = create_shelllink(policy, appname, aumi);
    return SUCCEEDED(hr) ? utility::shortcut_result::was_created : utility::shortcut_result::create_failed;
}

bool notification::init(notification_error* error) {
    is_initialized_ = false;
    if (shortcut_policy_ == utility::shortcut_policy::ignore) {
        if (is_enable_modern_features()) {
            set_error(error, notification_error::shell_link_not_created);
            return false;
        }
    }
    set_error(error, notification_error::no_error);
    if (aumi_.empty() || appname_.empty()) {
        set_error(error, notification_error::invalid_parameters);
        throw std::runtime_error("Error while initializing, did you set up a valid AUMI and App name?");
        return false;
    }
    if (static_cast<int>(utility::create_shortcut(shortcut_policy_, appname_, aumi_, has_winrt_initialized_)) < 0) {
        set_error(error, notification_error::shell_link_not_created);
        return false;
    }
    if (FAILED(rainy::set_current_process_aumi(aumi_))) {
        set_error(error, notification_error::invalid_app_user_model_id);
        throw std::runtime_error("Error while attaching the AUMI to the current proccess.");
        return false;
    }
    is_initialized_ = true;
    return is_initialized_;
}

bool notification::is_initialized() const {
    return is_initialized_;
}

std::wstring const& notification::app_name() const {
    return appname_;
}

std::wstring const& notification::app_user_model_id() const {
    return aumi_;
}

HRESULT utility::validate_shelllink(bool& was_changed,std::wstring_view appname,std::wstring_view aumi) {
    try {
        using namespace winrt::Windows::Storage;
        using namespace winrt::Windows::Foundation;
        WCHAR path[MAX_PATH] = { L'\0' };
        util::get_default_shell_link_path(appname, path);
        auto file = StorageFile::GetFileFromPathAsync(path).get();
        if (!file) {
            return E_FAIL;
        }

        auto properties = file.Properties().RetrievePropertiesAsync({ L"System.AppUserModel.ID" }).get();
        auto existingAumi = properties.Lookup(L"System.AppUserModel.ID");

        if (existingAumi && existingAumi.try_as<winrt::hstring>() && aumi != existingAumi.try_as<winrt::hstring>()) {
            was_changed = true;
            file.Properties().SavePropertiesAsync({ { L"System.AppUserModel.ID", winrt::box_value(aumi) } }).get();
        }
        else {
            was_changed = false;
        }
    }
    catch (const winrt::hresult_error& e) {
        return e.code();
    }
    return S_OK;
}

HRESULT utility::create_shelllink(shortcut_policy policy, std::wstring_view appname, std::wstring_view aumi) {
    if (policy != shortcut_policy::require_create) {
        return E_FAIL;
    }
    WCHAR exePath[MAX_PATH]{ L'\0' };
    WCHAR slPath[MAX_PATH]{ L'\0' };
    util::get_default_shell_link_path(appname, slPath);
    util::get_default_executable_path(exePath);
    // 创建快捷方式的 COM 接口操作
    winrt::com_ptr<IShellLinkW> shellLink;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, shellLink.put_void());
    if (FAILED(hr)) {
        return hr;
    }
    hr = shellLink->SetPath(exePath);
    if (FAILED(hr)) {
        return hr;
    }
    hr = shellLink->SetArguments(L"");
    if (FAILED(hr)) {
        return hr;
    }
    hr = shellLink->SetWorkingDirectory(exePath);
    if (FAILED(hr)) {
        return hr;
    }
	winrt::com_ptr<IPropertyStore> propertyStore;
	shellLink.as(propertyStore);
	PROPVARIANT appIdPropVar;
	hr = InitPropVariantFromString(aumi.data(), &appIdPropVar);
	if (FAILED(hr)) {
	    return hr;
	}
	hr = propertyStore->SetValue(PKEY_AppUserModel_ID, appIdPropVar);
	if (FAILED(hr)) {
	    return hr;
	}
	hr = propertyStore->Commit();
	if (FAILED(hr)) {
	    return hr;
	}
	// 使用 COM 保存快捷方式
    winrt::com_ptr<IPersistFile> persistFile;
    shellLink.as(persistFile);
    hr = persistFile->Save(slPath, TRUE);
    if (FAILED(hr)) {
        return hr;
    }
	PropVariantClear(&appIdPropVar);
    return hr;
}

static std::map<std::int64_t, std::shared_ptr<winrt::Windows::UI::Notifications::ToastNotification>> toast_mapper;

std::int64_t notification::show_impl(const notification_template& toast, std::shared_ptr<notification_handler> handler, notification_error* error) {
    set_error(error, notification_error::no_error);
    std::int64_t id = -1;
    HRESULT hr{ 0 };
    if (!is_initialized()) {
        set_error(error, notification_error::not_initialized);
        return id;
    }
    if (!handler) {
        set_error(error, notification_error::invalid_handler);
        return id;
    }
    using namespace winrt::Windows::UI::Notifications;
    using namespace winrt::Windows::Data::Xml::Dom;
    auto notifier = ToastNotificationManager::CreateToastNotifier(winrt::hstring{ aumi_ });
    utility::xml_notifcation_field xml(utility::xml_notifcation_field::context_bridge(*this), toast);
    auto notification = std::make_shared<ToastNotification>(xml);
    std::int64_t expiration = 0, relative_expiration = toast.expiration();
    if (relative_expiration > 0) {
        winrt::Windows::Foundation::DateTime now = winrt::clock::now(); // 将相对过期时间转换为时间点
        winrt::Windows::Foundation::DateTime expiration_time = now + std::chrono::milliseconds(relative_expiration);
        notification->ExpirationTime(expiration_time);
    }

    GUID guid;
    HRESULT hr_guid = CoCreateGuid(&guid);
    id = guid.Data1;
    winrt::event_token activated_token, dismissed_token, failed_token;
    winrt::check_hresult(hr);
    hr = util::set_event_handlers(*notification, handler, expiration, activated_token, dismissed_token, failed_token, [this, id]() { mark_as_ready_for_deletion(id); });
    if (FAILED(hr)) {
        set_error(error, notification_error::invalid_handler);
    }
    winrt::check_hresult(hr);
    notifier.Show(*notification);
    toast_mapper.insert({ id, notification });
    notifys.emplace(id, notify(*toast_mapper[id], activated_token, dismissed_token, failed_token));
    return FAILED(hr) ? -1 : id;
}

void notification::mark_as_ready_for_deletion(const std::int64_t id) {
    for (auto it = notifys.begin(); it != notifys.end();) {
        if (it->second.is_ready_for_deletion()) {
            it->second.remove_tokens();
            it = notifys.erase(it);
        } else {
            ++it;
        }
    }
    auto const iter = notifys.find(id);
    if (iter != notifys.end()) {
        notifys[id].mark_as_ready_for_deletion();
    }
}

bool notification::hide(const std::int64_t id) {
    if (!is_initialized()) {
        throw std::runtime_error("Error when hiding the toast. notification is not initialized.");
    }
    auto iter = notifys.find(id);
    if (iter == notifys.end()) {
        return false;
    }
    auto notifier = create_notifier();
    if (!notifier) {
        return false;
    }
    auto& notify = iter->second;
    notifier.value().Hide(notify.notification());
    notify.remove_tokens();
    notifys.erase(iter);
    const auto it = toast_mapper.find(id);
    if (it != toast_mapper.end()) {
        toast_mapper.erase(it);
    }
    return true;
}

void rainy::notification::set_modern_status(const bool enable) noexcept {
    enable_modern_features_ = enable;
}

bool rainy::notification::is_enable_modern_features() const noexcept {
    return enable_modern_features_;
}

void notification::clear() {
    auto notify = create_notifier();
    if (!notify) {
        return;
    }
    for (auto& data : notifys) {
        auto& notifyData = data.second;
        notify.value().Hide(notifyData.notification());
        notifyData.remove_tokens();
    }
    notifys.clear();
}

//
// Available as of Windows 10 Anniversary Update
// Ref: https://docs.microsoft.com/en-us/windows/uwp/design/shell/tiles-and-notifications/adaptive-interactive-toasts
//
// NOTE: This will add a new text field, so be aware when iterating over
//       the toast's text fields or getting a count of them.
//

HRESULT utility::xml_notifcation_field::set_attribution_text_field( std::wstring_view text) {
    util::create_element(xml, L"binding", L"text", { L"placement" });
    try {
        auto node_list = xml.GetElementsByTagName(L"text"); // 获取 text 节点列表
        for (uint32_t i = 0; i < node_list.Size(); ++i) {
            auto text_node = node_list.GetAt(i);
            auto attributes = text_node.Attributes();
            // 获取 "placement" 属性
            auto edited_node = attributes.GetNamedItem(L"placement");
            if (edited_node) {
                // 设置 attribution 字段
                util::set_node_string_value(L"attribution", &edited_node, xml);
                // 设置文本字段
                return set_text_field(text, i);
            }
        }
        return S_OK;
    }
    catch (const winrt::hresult_error& e) {
        return e.code();
    }
}

HRESULT utility::xml_notifcation_field::add_duration( std::wstring_view duration) {
    try {
        // 获取 toast 节点列表
        auto node_list = xml.GetElementsByTagName(L"toast");
        if (node_list.Size() > 0) {
            auto toast_node = node_list.GetAt(0);
            auto toast_element = toast_node.as<winrt::Windows::Data::Xml::Dom::XmlElement>();
            // 设置 duration 属性
            toast_element.SetAttribute(L"duration", duration);
        }
        return S_OK;
    }
    catch (const winrt::hresult_error& e) {
        return e.code();
    }
}

HRESULT utility::xml_notifcation_field::add_scenario( std::wstring_view scenario) {
    try {
        // 获取 toast 节点列表
        auto node_list = xml.GetElementsByTagName(L"toast");
        if (node_list.Size() > 0) {
            auto toast_node = node_list.GetAt(0);
            auto toast_element = toast_node.as<winrt::Windows::Data::Xml::Dom::XmlElement>();

            // 设置 scenario 属性
            toast_element.SetAttribute(L"scenario", scenario);
        }

        return S_OK;
    }
    catch (const winrt::hresult_error& e) {
        return e.code();
    }
}

HRESULT utility::xml_notifcation_field::add_input() {
    try {
        // 定义属性列表
        std::vector<std::wstring> attrbs = { L"id", L"type", L"placeHolderContent" };
        std::vector<std::wstring> attrbs2 = { L"content", L"arguments" };
        // 创建元素
        util::create_element(xml, L"toast", L"actions", {});
        util::create_element(xml, L"actions", L"input", attrbs);
        util::create_element(xml, L"actions", L"action", attrbs2);
        // 获取 "input" 节点列表
        auto node_list = xml.GetElementsByTagName(L"input");
        if (node_list.Size() > 0) {
            // 获取指定位置的节点
            auto input_node = node_list.GetAt(0);
            auto toast_element = input_node.as<winrt::Windows::Data::Xml::Dom::XmlElement>();
            // 设置属性
            toast_element.SetAttribute(L"id", L"textBox");
            toast_element.SetAttribute(L"type", L"text");
            toast_element.SetAttribute(L"placeHolderContent", L"...");
        }
        // 获取 "action" 节点列表
        auto node_list2 = xml.GetElementsByTagName(L"action");
        if (node_list2.Size() > 0) {
            // 获取指定位置的节点
            auto action_node = node_list2.GetAt(0);
            auto action_element = action_node.as<winrt::Windows::Data::Xml::Dom::XmlElement>();
            // 设置属性
            action_element.SetAttribute(L"content", L"Reply");
            action_element.SetAttribute(L"arguments", L"action=reply");
            action_element.SetAttribute(L"hint-inputId", L"textBox");
        }
        return S_OK;
    }
    catch (const winrt::hresult_error& e) {
        return e.code();
    }
}

HRESULT utility::xml_notifcation_field::set_text_field( std::wstring_view text, std::uint32_t pos) {
    try {
        // 获取 "text" 节点列表
        auto node_list = xml.GetElementsByTagName(L"text");
        if (node_list.Size() > pos) {
            // 获取指定位置的节点
            auto node = node_list.GetAt(pos);
            return util::set_node_string_value(text, &node, xml); // 设置节点的字符串值
        }
        return E_FAIL;
    }
    catch (const winrt::hresult_error& e) {
        return e.code();
    }
}

HRESULT utility::xml_notifcation_field::set_bind_toast_generic() {
    try {
        // 获取 "binding" 节点列表
        auto nodeList = xml.GetElementsByTagName(L"binding");
        if (nodeList.Size() > 0) {
            // 获取第一个节点
            auto toastNode = nodeList.GetAt(0);
            // 获取节点并设置模板属性
            if (auto toastElement = toastNode.as<winrt::Windows::Data::Xml::Dom::XmlElement>()) {
                toastElement.SetAttribute(L"template", L"ToastGeneric");
            }
            return S_OK;
        }
        return E_FAIL;
    }
    catch (const winrt::hresult_error& e) {
        return e.code();
    }
}

std::optional<winrt::Windows::UI::Notifications::ToastNotifier> notification::create_notifier() const {
    try {
        auto notificationManager = winrt::Windows::UI::Notifications::ToastNotificationManager::GetDefault();
        return notificationManager.CreateToastNotifier(aumi_);
    }
    catch (const winrt::hresult_error&) {
        return std::nullopt;
    }
}

rainy::utility::xml_notifcation_field::xml_notifcation_field(context_bridge ctx_bridge,
                                                             const notification_template &notifcation_template) {
    using namespace winrt::Windows::UI::Notifications;
    using namespace winrt::Windows::Data::Xml::Dom;
    xml = ToastNotificationManager::GetTemplateContent(static_cast<ToastTemplateType>(notifcation_template.template_type()));
    HRESULT hr = S_OK;
    if (notifcation_template.is_toast_generic()) {
        hr = set_bind_toast_generic();
    }
    for (std::uint32_t i = 0, fields_count = static_cast<std::uint32_t>(notifcation_template.text_fields_count());
         i < fields_count && SUCCEEDED(hr); i++) {
        hr = set_text_field(notifcation_template.text_field(notification_template::textfield(i)), i);
    }
    winrt::check_hresult(hr);
    if (ctx_bridge.is_supporting_modern_features() && ctx_bridge.is_enable_modern_features()) {
        if (!notifcation_template.attribution_text().empty()) {
            hr = set_attribution_text_field(notifcation_template.attribution_text());
        }
        if (notifcation_template.has_input() && notifcation_template.actions.count() != 0) {
            hr = add_input(); // 仅允许add_input
        } else {
            std::array<WCHAR, 12> buf{};
            for (std::size_t i = 0, actions_count = notifcation_template.actions.count(); i < actions_count && SUCCEEDED(hr); i++) {
                _snwprintf_s(buf.data(), buf.size(), _TRUNCATE, L"%zd", i);
                hr = add_action(notifcation_template.actions.action_label(i), buf.data());
                winrt::check_hresult(hr);
            }
            if (notifcation_template.has_input()) {
                hr = add_input();
            }
            winrt::check_hresult(hr);
        }
        hr = (notifcation_template.audio_path().empty() &&
              notifcation_template.audio_option() == notification_template::audio_option_t::default_option)
                 ? hr
                 : set_audio_field(notifcation_template.audio_path(), notifcation_template.audio_option());
        winrt::check_hresult(hr);
        auto duration = notifcation_template.duration();
        if (duration != notification_template::duration_t::system) {
            hr = add_duration((duration == notification_template::duration_t::short_duration) ? L"short" : L"long");
        }
        winrt::check_hresult(hr);
        hr = add_scenario(notifcation_template.scenario());
    }
    winrt::check_hresult(hr);
    const bool is_win10_anniversary_or_above = notification::is_win10_anniversary_or_higher();
    const bool is_circle_crop_hint = is_win10_anniversary_or_above ? notifcation_template.is_crop_hint_circle() : false;
    hr = notifcation_template.has_image()
             ? set_image_field(notifcation_template.image_path(), notifcation_template.is_toast_generic(), is_circle_crop_hint)
             : hr;
    winrt::check_hresult(hr);
    if (is_win10_anniversary_or_above && notifcation_template.has_hero_image()) {
        hr = set_hero_image(notifcation_template.hero_image_path(), notifcation_template.is_inline_hero_image());
    }
    winrt::check_hresult(hr);
}

HRESULT utility::xml_notifcation_field::set_image_field(
    std::wstring_view path,
    bool is_toast_generic,
    bool is_crop_hint_circle) {
    try {
        // 确保路径大小小于最大路径
        assert(path.size() < MAX_PATH);
        // 构建完整的文件路径
        std::wstring image_path = L"file:///";
        image_path += path.data();
        // 获取 "image" 节点列表
        auto node_list = xml.GetElementsByTagName(L"image");
        if (node_list.Size() > 0) {
            // 获取第一个节点
            auto node = node_list.GetAt(0);
            auto image_element = node.as<winrt::Windows::Data::Xml::Dom::XmlElement>();
            // 如果是 toast_generic 且节点有效，设置属性
            if (is_toast_generic) {
                image_element.SetAttribute(L"placement", L"app_logo_override");
                // 如果是圆形裁剪提示，设置 hint_crop 属性
                if (is_crop_hint_circle) {
                    image_element.SetAttribute(L"hint_crop", L"circle");
                }
            }
            // 获取属性节点，并修改 "src" 属性
            auto attributes = node.Attributes();
            auto edited_node = attributes.GetNamedItem(L"src");
            if (edited_node != nullptr) {
                util::set_node_string_value(image_path, &edited_node, xml);
            }
        }
        return S_OK;
    }
    catch (const winrt::hresult_error& e) {
        return e.code();
    }
}

HRESULT utility::xml_notifcation_field::set_audio_field(
    std::wstring_view path,
    notification_template::audio_option_t option) {
    try {
        std::vector<std::wstring> attrs;
        if (!path.empty()) {
            attrs.push_back(L"src");
        }
        if (option == notification_template::audio_option_t::loop) {
            attrs.push_back(L"loop");
        }
        if (option == notification_template::audio_option_t::silent) {
            attrs.push_back(L"silent");
        }
        // 创建 audio 元素
        util::create_element(xml, L"toast", L"audio", attrs);
        // 获取 "audio" 节点列表
        auto node_list = xml.GetElementsByTagName(L"audio");
        if (node_list.Size() > 0) {
            // 获取第一个节点
            auto node = node_list.GetAt(0);
            auto attributes = node.Attributes();
            // 如果提供了路径，设置 src 属性
            if (!path.empty()) {
                auto edited_node = attributes.GetNamedItem(L"src");
                if (edited_node != nullptr) {
                    util::set_node_string_value(path, &edited_node, xml);
                }
            }
            // 根据选项设置 audio 属性
            switch (option) {
            case notification_template::audio_option_t::loop: {
                auto edited_node = attributes.GetNamedItem(L"loop");
                if (edited_node != nullptr) {
                    util::set_node_string_value(L"true", &edited_node, xml);
                }
                break;
            }
            case notification_template::audio_option_t::silent: {
                auto edited_node = attributes.GetNamedItem(L"silent");
                if (edited_node != nullptr) {
                    util::set_node_string_value(L"true", &edited_node, xml);
                }
                break;
            }
            default:
                break;
            }
        }
        return S_OK;
    }
    catch (const winrt::hresult_error& e) {
        return e.code();
    }
}

HRESULT utility::xml_notifcation_field::add_action(
    std::wstring_view content,
    std::wstring_view arguments) {
    try {
        // 获取 actions 节点列表
        auto node_list = xml.GetElementsByTagName(L"actions");
        // 判断 actions 节点是否存在
        if (node_list.Size() > 0) {
            auto actions_node = node_list.GetAt(0);
            // 创建并设置 action 元素
            auto action_element = xml.CreateElement(L"action");
            action_element.SetAttribute(L"content", content);
            action_element.SetAttribute(L"arguments", arguments);
            // 将 action 元素添加到 actions 节点
            actions_node.AppendChild(action_element);
        }
        else {
            // 如果没有 actions 节点，创建一个新的 actions 节点
            node_list = xml.GetElementsByTagName(L"toast");
            if (node_list.Size() > 0) {
                auto toast_node = node_list.GetAt(0);
                auto toast_element = toast_node.as<winrt::Windows::Data::Xml::Dom::XmlElement>();
                // 设置 Toast 元素的模板和持续时间
                toast_element.SetAttribute(L"template", L"ToastGeneric");
                toast_element.SetAttribute(L"duration", L"long");
                // 创建 actions 元素并添加到 toast 节点
                auto actions_element = xml.CreateElement(L"actions");
                toast_node.AppendChild(actions_element);
                // 创建并设置 action 元素
                auto action_element = xml.CreateElement(L"action");
                action_element.SetAttribute(L"content", content);
                action_element.SetAttribute(L"arguments", arguments);
                // 将 action 元素添加到 actions 节点
                actions_element.AppendChild(action_element);
            }
        }
        return S_OK;
    }
    catch (const winrt::hresult_error& e) {
        return e.code();
    }
}

HRESULT utility::xml_notifcation_field::set_hero_image( std::wstring_view path, bool is_inline_image) {
    try {
        // 获取 "binding" 节点列表
        auto node_list = xml.GetElementsByTagName(L"binding");
        if (node_list.Size() > 0) {
            // 获取第一个节点
            auto binding_node = node_list.GetAt(0);
            // 创建 "image" 元素
            auto image_element = xml.CreateElement(L"image");
            // 如果不是内联图片，设置 placement 属性
            if (!is_inline_image) {
                image_element.SetAttribute(L"placement", L"hero");
            }
            image_element.SetAttribute(L"src", path); // 设置 src 属性为指定路径
            binding_node.AppendChild(image_element); // 将 "image" 元素附加到 "binding" 节点
            return S_OK;
        }
        return E_FAIL;
    }
    catch (const winrt::hresult_error& e) {
        return e.code();
    }
}

void notification::set_error(notification_error* error, notification_error value) {
    if (error) {
        *error = value;
    }
}