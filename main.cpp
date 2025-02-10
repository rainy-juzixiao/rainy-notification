#include "notification.hpp"

int main() {
    rainy::notification notification;
	notification.set_app_name(L"rainy's app");
	notification.set_aumi(L"rainy's app");
	notification.init();
	rainy::notification_template notification_template;
	notification_template.set_first_line(L"你好");
    notification_template.toggle_input();
    notification_template.actions.add_action(L"你好");
    notification.show(notification_template, [](const rainy::notification_event &e) {
        if (e.type == rainy::notification_event::event_type::activated) {
            std::wcout << L"激活！" << std::endl;
        } else if (e.type == rainy::notification_event::event_type::activated_with_reply) {
            std::wcout << L"激活信息附带如下：" << std::get<std::wstring_view>(e.data) << "\n";
        }
    });
	system("pause");
	return 0;
}