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
#ifndef RAINY_WINAPI_NOTIFICATION_HPP
#define RAINY_WINAPI_NOTIFICATION_HPP
#include <Windows.h>
#include <ShObjIdl.h>
#include <strsafe.h>
#include <Psapi.h>
#include <ShlObj.h>
#include <roapi.h>
#include <propvarutil.h>
#include <functiondiscoverykeys.h>
#include <iostream>
#include <winstring.h>
#include <string.h>
#include <variant>
#include <vector>
#include <winrt/windows.storage.h>
#include <winrt/windows.data.xml.dom.h>
#include <winrt/windows.ui.notifications.h>
#include <winrt/windows.storage.fileproperties.h>
#include <winrt/windows.foundation.collections.h>

#define RAINY_NODISCARD [[nodiscard]]

namespace rainy::internals {
    using abi_toast_dismissal_reason = winrt::Windows::UI::Notifications::ToastDismissalReason;
    using abi_template_type = winrt::Windows::UI::Notifications::ToastTemplateType;
    constexpr static std::size_t text_fields_count[] = { 1, 2, 2, 3, 1, 2, 2, 3 };
}

namespace rainy {
    enum class notification_template_type {
        image_and_text01 = internals::abi_template_type::ToastImageAndText01,
        image_and_text02 = internals::abi_template_type::ToastImageAndText02,
        image_and_text03 = internals::abi_template_type::ToastImageAndText03,
        image_and_text04 = internals::abi_template_type::ToastImageAndText04,
        text01 = internals::abi_template_type::ToastText01,
        text02 = internals::abi_template_type::ToastText02,
        text03 = internals::abi_template_type::ToastText03,
        text04 = internals::abi_template_type::ToastText04
    };
}

namespace rainy {
    class notification_template {
    public:
        enum class scenario_t {
            normal,
            alarm,
            incoming_call,
            reminder
        };

        enum class duration_t {
            system,
            short_duration,
            long_duration
        };

        enum class audio_option_t {
            default_option,
            silent,
            loop
        };

        enum class textfield {
            first_line,
            second_line,
            third_line
        };

        enum class crop_hint {
            square,
            circle
        };

        enum class audio_system_file {
            default_sound,
            im,
            mail,
            reminder,
            sms,
            alarm,
            alarm2,
            alarm3,
            alarm4,
            alarm5,
            alarm6,
            alarm7,
            alarm8,
            alarm9,
            alarm10,
            call,
            call1,
            call2,
            call3,
            call4,
            call5,
            call6,
            call7,
            call8,
            call9,
            call10,
        };

        class actions_t {
        public:
            actions_t(notification_template *this_) : this_(this_) {
            }

            const auto &get_container() const noexcept {
                return data;
            }

            /**
             * @brief 获取通知模板中的操作按钮的标签的引用，按位置索引
             * @param pos 指定的位置索引
             * @return 对应的位置索引的操作按钮的标签的引用
             */
            const std::wstring_view &action_label(const std::size_t pos) const noexcept {
                return data.at(pos);
            }

            /**
             * @brief 添加一个操作标签，最多支持5个标签
             * @param label 标签内容
             */
            void add_action(std::wstring_view label) noexcept {
                if (!this_->has_input() && actions_count_ != 5) {
                    data[actions_count_++] = label;
                }
            }

            /**
             * @brief 批量添加操作标签，最多支持5个标签
             * @param ilist 初始化列表
             */
            void add_action(std::initializer_list<std::wstring_view> ilist) noexcept {
                for (const auto &label: ilist) {
                    if (actions_count_ == 5) {
                        break;
                    }
                    data[actions_count_++] = label;
                }
            }

            /**
             * @brief 检查当前是否没有操作标签
             * @return true 如果没有任何操作标签
             */
            bool empty() const noexcept {
                return actions_count_ == 0;
            }

            /**
             * @brief 获取当前已添加的操作标签数量
             * @return 当前操作标签的数量
             */
            std::size_t count() const noexcept {
                return actions_count_;
            }

            /**
             * @brief 删除指定位置的操作标签
             * @param pos 标签的位置索引
             * @return true 如果删除成功
             */
            bool remove_action(const std::size_t pos) noexcept {
                if (pos >= actions_count_) {
                    return false;
                }
                for (std::size_t i = pos; i < actions_count_ - 1; ++i) {
                    data[i] = data[i + 1];
                }
                --actions_count_;
                return true;
            }

            /**
             * @brief 清空所有操作标签
             */
            void clear() noexcept {
                actions_count_ = 0;
            }

            /**
             * @brief 设置指定位置的操作标签
             * @param pos 标签的位置索引
             * @param label 新的标签内容
             * @return true 如果设置成功
             */
            bool set_action_label(const std::size_t pos, std::wstring_view label) noexcept {
                if (pos >= actions_count_) {
                    return false;
                }
                data[pos] = label;
                return true;
            }

        private:
            std::size_t actions_count_{0};
            std::array<std::wstring_view, 5> data{};
            notification_template *this_;
        };

        notification_template() : notification_template(notification_template_type::text01) {
        }

        notification_template(notification_template_type type) : template_type_(type), has_input_(false), actions(this) {
        }

        void set_first_line(std::wstring_view text) noexcept {
            set_text_field(text, textfield::first_line);
        }

        void set_second_line(std::wstring_view text) noexcept {
            set_text_field(text, textfield::second_line);
        }

        void set_third_line(std::wstring_view text) noexcept {
            set_text_field(text, textfield::third_line);
        }

        void set_text_field(std::wstring_view text, textfield pos) noexcept {
            const auto position = static_cast<std::size_t>(pos);
            if (internals::text_fields_count[static_cast<std::size_t>(template_type_)] < position) {
                return;
            }
            text_fields_[position] = text;
        }

        void attribution_text(std::wstring_view attribution_text) {
            attribution_text_ = attribution_text;
        }

        void image_path(std::wstring_view img_path, crop_hint crop_hint = crop_hint::square) {
            image_path_ = img_path;
            crop_hint_ = crop_hint;
        }

        void hero_image_path(std::wstring_view img_path, bool inline_image = false) {
            hero_image_path_ = img_path;
            inline_hero_image = inline_image;
        }

        void audio_path(audio_system_file audio) {
            static const std::unordered_map<audio_system_file, std::wstring> preset_audio_files = {
                {audio_system_file::default_sound, L"ms-winsoundevent:Notification.Default"},
                {audio_system_file::im, L"ms-winsoundevent:Notification.IM"},
                {audio_system_file::mail, L"ms-winsoundevent:Notification.Mail"},
                {audio_system_file::reminder, L"ms-winsoundevent:Notification.Reminder"},
                {audio_system_file::sms, L"ms-winsoundevent:Notification.SMS"},
                {audio_system_file::alarm, L"ms-winsoundevent:Notification.Looping.Alarm"},
                {audio_system_file::alarm2, L"ms-winsoundevent:Notification.Looping.Alarm2"},
                {audio_system_file::alarm3, L"ms-winsoundevent:Notification.Looping.Alarm3"},
                {audio_system_file::alarm4, L"ms-winsoundevent:Notification.Looping.Alarm4"},
                {audio_system_file::alarm5, L"ms-winsoundevent:Notification.Looping.Alarm5"},
                {audio_system_file::alarm6, L"ms-winsoundevent:Notification.Looping.Alarm6"},
                {audio_system_file::alarm7, L"ms-winsoundevent:Notification.Looping.Alarm7"},
                {audio_system_file::alarm8, L"ms-winsoundevent:Notification.Looping.Alarm8"},
                {audio_system_file::alarm9, L"ms-winsoundevent:Notification.Looping.Alarm9"},
                {audio_system_file::alarm10, L"ms-winsoundevent:Notification.Looping.Alarm10"},
                {audio_system_file::call, L"ms-winsoundevent:Notification.Looping.Call"},
                {audio_system_file::call1, L"ms-winsoundevent:Notification.Looping.Call1"},
                {audio_system_file::call2, L"ms-winsoundevent:Notification.Looping.Call2"},
                {audio_system_file::call3, L"ms-winsoundevent:Notification.Looping.Call3"},
                {audio_system_file::call4, L"ms-winsoundevent:Notification.Looping.Call4"},
                {audio_system_file::call5, L"ms-winsoundevent:Notification.Looping.Call5"},
                {audio_system_file::call6, L"ms-winsoundevent:Notification.Looping.Call6"},
                {audio_system_file::call7, L"ms-winsoundevent:Notification.Looping.Call7"},
                {audio_system_file::call8, L"ms-winsoundevent:Notification.Looping.Call8"},
                {audio_system_file::call9, L"ms-winsoundevent:Notification.Looping.Call9"},
                {audio_system_file::call10, L"ms-winsoundevent:Notification.Looping.Call10"},
            };
            const auto get = preset_audio_files.find(audio);
            if (get == preset_audio_files.end()) {
                audio_path_ = preset_audio_files.at(audio_system_file::default_sound);
            } else {
                audio_path_ = get->second;
            }
        }

        void audio_path(std::wstring_view audio_path) {
            audio_path_ = audio_path;
        }

        void audio_option(audio_option_t audio_option) {
            audio_option_ = audio_option;
        }

        std::wstring_view scenario() noexcept {
            return scenario_;
        }

        void scenario(scenario_t scenario) noexcept {
            switch (scenario) {
                case scenario_t::normal:
                    scenario_ = L"Default";
                    break;
                case scenario_t::alarm:
                    scenario_ = L"Alarm";
                    break;
                case scenario_t::incoming_call:
                    scenario_ = L"IncomingCall";
                    break;
                case scenario_t::reminder:
                    scenario_ = L"Reminder";
                    break;
            }
        }

        void toggle_input(const bool enable = true) noexcept {
            if (actions.empty()) {
                has_input_ = true;
            }
        }

        bool has_input() const noexcept {
            return has_input_;
        }

        /**
         * @brief 获取通知字段中的文本行数
         * @return 通知字段中的文本行数
         */
        RAINY_NODISCARD std::size_t text_fields_count() const noexcept {
            return internals::text_fields_count[static_cast<int>(template_type())];
        }

        /**
         * @brief 检索通知模板是否包含图片
         * @return 指示通知模板是否包含图片
         */
        RAINY_NODISCARD bool has_image() const noexcept {
            return template_type_ < notification_template_type::text01;
        }

        /**
         * @brief 检索通知模板是否包含Hero Image
         * @return 指示通知模板是否包含Hero Image
         */
        RAINY_NODISCARD bool has_hero_image() const noexcept {
            return !hero_image_path_.empty();
        }

        /**
         * @brief 获取通知模板的文本字段的引用
         * @return 通知模板的文本字段的引用
         */
        RAINY_NODISCARD const auto &text_fields() const noexcept {
            return text_fields_;
        }

        /**
         * @brief 获取通知模板的操作按钮的标签的引用，按位置索引
         * @param pos 指定行数的位置
         * @return 对应行数的操作按钮的标签的引用
         */
        RAINY_NODISCARD std::wstring_view text_field(const textfield pos) const {
            return text_fields_.at(static_cast<int>(pos));
        }

        /**
         * @brief 获取通知模板的图像路径
         * @return 返回图像的路径
         */
        RAINY_NODISCARD std::wstring_view image_path() const {
            return image_path_;
        }

        /**
         * @brief 获取Hero Image的路径
         * @return 返回Hero Image的路径
         */
        RAINY_NODISCARD std::wstring_view hero_image_path() const {
            return hero_image_path_;
        }

        /**
         * @brief 获取通知模板的音频路径
         * @return 返回音频的路径
         */
        RAINY_NODISCARD std::wstring_view audio_path() const {
            return audio_path_;
        }

        /**
         * @brief 获取通知模板的归属信息
         * @return 返回通知模板的归属信息
         */
        RAINY_NODISCARD std::wstring_view attribution_text() const {
            return attribution_text_;
        }

        /**
         * @brief 获取通知模板的场景
         * @return 获取通知模板的场景
         */
        RAINY_NODISCARD std::wstring_view scenario() const noexcept {
            return scenario_;
        }

        /**
         * @brief 获取通知中不再被视为当前或有效且不应显示的时间
         * @return 以std::int64_t的形式返回，单位为毫秒
         */
        RAINY_NODISCARD std::int64_t expiration() const noexcept {
            return expiration_;
        }

        void expiration(const std::int64_t milliseconds_from_now) noexcept {
            expiration_ = milliseconds_from_now;
        }

        /**
         * @brief 获取通知模板使用的类型
         * @return 返回一个枚举值，表示通知模板的类型
         */
        RAINY_NODISCARD notification_template_type template_type() const {
            return template_type_;
        }

        /**
         * @brief 检索音频选项
         * @return 返回一个枚举值，表示音频选项
         */
        RAINY_NODISCARD audio_option_t audio_option() const {
            return audio_option_;
        }

        /**
         * @brief 获取通知显示的时间
         * @return 返回一个枚举值进行表示
         */
        RAINY_NODISCARD duration_t duration() const noexcept {
            return duration_;
        }

        void duration(duration_t duration) noexcept {
            duration_ = duration;
        }

        /**
         * @brief 检查当前模板是否为通用模板
         * @return 指示是否为通用模板
         */
        RAINY_NODISCARD bool is_toast_generic() const noexcept {
            return has_hero_image() || crop_hint_ == crop_hint::circle;
        }


        RAINY_NODISCARD bool is_inline_hero_image() const noexcept {
            return inline_hero_image;
        }


        RAINY_NODISCARD bool is_crop_hint_circle() const noexcept {
            return crop_hint_ == crop_hint::circle;
        }

        actions_t actions;

    private:
        bool has_input_;
        bool inline_hero_image{false};
        std::int64_t expiration_{0};
        std::array<std::wstring, 3> text_fields_{};
        std::wstring image_path_{};
        std::wstring hero_image_path_{};
        std::wstring audio_path_{};
        std::wstring attribution_text_{};
        std::wstring scenario_{L"Default"};
        audio_option_t audio_option_{audio_option_t::default_option};
        notification_template_type template_type_{notification_template_type::text01};
        duration_t duration_{duration_t::system};
        crop_hint crop_hint_{crop_hint::square};
    };
}

namespace rainy {
    struct notification_handler {
        enum class dismissal_reason {
            user_canceled = internals::abi_toast_dismissal_reason::UserCanceled,
            application_hidden = internals::abi_toast_dismissal_reason::ApplicationHidden,
            timed_out = internals::abi_toast_dismissal_reason::TimedOut
        };

        virtual ~notification_handler() = default;
        virtual void activated() const = 0;
        virtual void activated(int action_idx) const = 0;
        virtual void activated(const std::wstring_view response) const = 0;
        virtual void dismissed(dismissal_reason state) const = 0;
        virtual void failed() const = 0;
    };

    struct mono_notification_handler_t final : notification_handler {
        ~mono_notification_handler_t() = default;
        void activated() const override {
        }
        void activated(int action_idx) const override {
        }
        void activated(const std::wstring_view response) const override {
        }
        void dismissed(dismissal_reason state) const override {
        }
        void failed() const override {
        }
    };

    static const mono_notification_handler_t mono_notification_handler;

    struct notification_event {
        enum class event_type {
            activated,
            activated_with_action_idx,
            activated_with_reply,
            dismissed,
            failed
        };

        event_type type;
        std::variant<std::wstring_view, notification_handler::dismissal_reason, int, std::monostate> data;
    };
}

namespace rainy::utility {
    class xml_notifcation_field {
    public:
        struct context_bridge {
            struct state {
                static constexpr int placeholder = 0;
                static constexpr int is_supporting_modern_features = 2;
                static constexpr int is_enable_modern_features = 4;
                static constexpr int is_win10_anniversary_or_higher = 8;
            };

            template <typename Notifcation>
            context_bridge(Notifcation &context) : option() {
                if (context.is_supporting_modern_features()) {
                    option |= state::is_supporting_modern_features;
                }
                if (context.is_enable_modern_features()) {
                    option |= state::is_enable_modern_features;
                }
                if (context.is_win10_anniversary_or_higher()) {
                    option |= state::is_win10_anniversary_or_higher;
                }
            }

            bool is_supporting_modern_features() const noexcept {
                return option & state::is_supporting_modern_features;
            }

            bool is_enable_modern_features() const noexcept {
                return option & state::is_enable_modern_features;
            }

            bool is_win10_anniversary_or_higher() const noexcept {
                return option & state::is_win10_anniversary_or_higher;
            }

        private:
            int option;
        };

        xml_notifcation_field(context_bridge ctx_bridge,const notification_template& notifcation_template);

        operator winrt::Windows::Data::Xml::Dom::XmlDocument &() noexcept {
            return xml;
        }

        void load_xml(const std::wstring_view xml_view) {
            if (!xml_view.empty()) {
                xml.LoadXml(winrt::hstring{xml_view});
            }
        }

        HRESULT set_image_field(std::wstring_view path, bool is_toast_generic, bool is_crop_hint_circle);
        HRESULT set_hero_image(std::wstring_view path, bool is_inline_image);
        HRESULT set_bind_toast_generic();
        HRESULT set_audio_field(std::wstring_view path,
                                notification_template::audio_option_t option = notification_template::audio_option_t::default_option);
        HRESULT set_text_field(std::wstring_view text, std::uint32_t pos);
        HRESULT set_attribution_text_field(std::wstring_view text);
        HRESULT add_action(std::wstring_view action, std::wstring_view arguments);
        HRESULT add_duration(std::wstring_view duration);
        HRESULT add_scenario(std::wstring_view scenario);
        HRESULT add_input();

    private:
        winrt::Windows::Data::Xml::Dom::XmlDocument xml;
    };
}

namespace rainy::utility {
    enum class shortcut_result {
        unchanged = 0,
        was_changed = 1,
        was_created = 2,
        missing_parameters = -1,
        inis_compatible_os = -2,
        com_init_failure = -3,
        create_failed = -4
    };

    enum class shortcut_policy {
        ignore = 0,
        require_no_create = 1,
        require_create = 2,
    };

    HRESULT create_shelllink(shortcut_policy policy, std::wstring_view appname, std::wstring_view aumi);
    HRESULT validate_shelllink(bool &was_changed, std::wstring_view appname, std::wstring_view aumi);

    shortcut_result create_shortcut(shortcut_policy policy, std::wstring_view appname, std::wstring_view aumi, bool &winrt_init_flag);
}

namespace rainy {
    enum class notification_error {
        no_error,
        not_initialized,
        system_not_supported,
        shell_link_not_created,
        invalid_app_user_model_id,
        invalid_parameters,
        invalid_handler,
        not_displayed,
        unknown_error
    };

    class notification {
    public:
        notification();
        ~notification();
        static bool is_supporting_modern_features();
        static bool is_win10_anniversary_or_higher();
        static std::wstring make_aumi(std::wstring const &company_name, std::wstring const &product_name,
                                      std::wstring const &sub_product = {}, std::wstring const &version_information = {});
        static std::wstring const &strerror(notification_error error);
        bool init(notification_error *error = nullptr);
        bool is_initialized() const;
        bool hide(const std::int64_t id);
        void set_modern_status(const bool enable) noexcept;
        bool is_enable_modern_features() const noexcept;
        void clear();
        std::wstring const &app_name() const;
        std::wstring const &app_user_model_id() const;
        void set_aumi(std::wstring_view aumi);
        void set_app_name(std::wstring const &app_name);
        void set_shortcut_policy(utility::shortcut_policy policy);

        template <typename EventHandler, std::enable_if_t<std::is_base_of_v<notification_handler, EventHandler>, int> = 0>
        std::int64_t show(const notification_template &notification, notification_error *error = nullptr) {
            try {
                return show_impl(notification, std::make_shared<EventHandler>(), error);
            } catch (const winrt::hresult_error &) {
                return -1;
            }
        }

        template <typename EventHandler, std::enable_if_t<std::is_base_of_v<notification_handler, EventHandler>, int> = 0>
        std::int64_t show(const notification_template &notification, EventHandler &handler, notification_error *error = nullptr) {
            try {
                return show_impl(notification, &handler, error);
            } catch (const winrt::hresult_error &) {
                return -1;
            }
        }

        template <typename EventHandler,
                  typename = std::void_t<decltype(std::declval<EventHandler>()(std::declval<const rainy::notification_event &>()))>>
        std::int64_t show(const notification_template &notification, EventHandler handler, notification_error *error = nullptr) {
            struct unnamed_handler final : EventHandler, notification_handler {
                using event_t = notification_event;

                unnamed_handler(EventHandler &&handler) : EventHandler(handler) {
                }

                void activated() const override {
                    event_t event{event_t::event_type::activated, {}};
                    call_handler(event);
                }

                void activated(int action_idx) const override {
                    event_t event{event_t::event_type::activated_with_action_idx, action_idx};
                    call_handler(event);
                }

                void activated(const std::wstring_view response) const override {
                    event_t event{event_t::event_type::activated_with_reply, response};
                    call_handler(event);
                }

                void dismissed(dismissal_reason state) const override {
                    event_t event{event_t::event_type::dismissed, state};
                    call_handler(event);
                }

                void failed() const override {
                    event_t event{event_t::event_type::failed, {}};
                    call_handler(event);
                }

                void call_handler(const event_t &event) const {
                    (*this)(event);
                }
            };
            std::shared_ptr<notification_handler> ptr_handler = std::make_shared<unnamed_handler>(std::forward<EventHandler>(handler));
            try {
                return show_impl(notification, ptr_handler, error);
            } catch (const winrt::hresult_error &) {
                return -1;
            }
        }

    protected:
        std::int64_t show_impl(notification_template const &notification, std::shared_ptr<notification_handler> event_handler,
                               notification_error *error);
        struct notify {
            notify() {};
            notify(const winrt::Windows::UI::Notifications::ToastNotification &notify, winrt::event_token activated_token,
                   winrt::event_token dismissed_token, winrt::event_token failed_token) :
                _notify(notify), _activated_token(activated_token), _dismissed_token(dismissed_token), _failed_token(failed_token) {
            }

            ~notify() {
                remove_tokens();
            }

            void remove_tokens() {
                if (!_ready_for_deletion) {
                    return;
                }

                if (_previously_token_removed) {
                    return;
                }

                if (!_notify) {
                    return;
                }
                _notify.Activated(_activated_token);
                _notify.Dismissed(_dismissed_token);
                _notify.Failed(_failed_token);

                _previously_token_removed = true;
            }

            void mark_as_ready_for_deletion() {
                _ready_for_deletion = true;
            }

            bool is_ready_for_deletion() const {
                return _ready_for_deletion;
            }

            const winrt::Windows::UI::Notifications::ToastNotification &notification() {
                return _notify;
            }

        private:
            const winrt::Windows::UI::Notifications::ToastNotification &_notify{nullptr};
            winrt::event_token _activated_token{};
            winrt::event_token _dismissed_token{};
            winrt::event_token _failed_token{};
            bool _ready_for_deletion{false};
            bool _previously_token_removed{false};
        };

        bool is_initialized_{false};
        bool has_winrt_initialized_{false};
        bool enable_modern_features_{true};
        utility::shortcut_policy shortcut_policy_{utility::shortcut_policy::require_create};
        std::wstring appname_{};
        std::wstring aumi_{};
        std::map<std::int64_t, notify> notifys{};

        void mark_as_ready_for_deletion(const std::int64_t id);

        std::optional<winrt::Windows::UI::Notifications::ToastNotifier> create_notifier() const;
        void set_error(notification_error *error, notification_error value);
    };
}

#endif
