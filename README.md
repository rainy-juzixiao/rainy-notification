# rainy-notification

## 中文（CHS/Chinese）

### 介绍

这只是一个用C++17标准编写的WinRT程序库。它的功能很简单，仅仅只是用于调用Windows的通知组件

此库是对WinToast库的重写，以WinRT的形式进行提供。同时，修正一些来自WinToast库的细节问题。

当然，可以访问下面的网址查看WinToast项目

https://github.com/mohabouje/WinToast

本库支持CMake系统构建。请确保标准必须满足`C++ 17`（WinRT要求在`C++ 17`标准中才能工作）

### 注意

本库的开源许可证与WinToast的并不会一致。采用Apache 2.0进行分发，而不会采用MIT。请在此注意。因为所有编写的源代码被WinRT重写。因此，它不会采用

后面还会持续更新。我尽量维护吧

### 如何使用

本库已经为你提供了注释文档。但是它是以中文形式提供的。但是，从函数名上，大概也能看出用途。另外，除了模板外，所有命名使用snake_case

下面仅仅只是一个演示demo，不过也演示了一些基本内容

```cpp
#include "include/rainy_notification.hpp"

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
```

## English

### Introduction

This is a WinRT library written in C++17. Its functionality is quite simple—it is just used to call the Windows Notification component.

This library is a reimplementation of the WinToast library, provided in the form of WinRT. It also fixes some of the detail issues from the original WinToast library.

For more information on WinToast, you can visit:

https://github.com/mohabouje/WinToast

This library supports CMake build system. Please ensure that your project is built with `C++ 17`, as WinRT requires `C++ 17` to work properly.

### How to Use
This library already provides annotated documentation, but it's in Chinese. However, from the function names, you can generally infer their purpose. Additionally, all names, except for templates, follow the snake_case naming convention.

The following is just a demo, but it also demonstrates some basic features.

```cpp
#include "include/rainy_notification.hpp"

int main() {
    rainy::notification notification;
	notification.set_app_name(L"rainy's app");
	notification.set_aumi(L"rainy's app");
	notification.init();
	rainy::notification_template notification_template;
	notification_template.set_first_line(L"Hello");
    notification_template.toggle_input();
    notification_template.actions.add_action(L"Hello");
    notification.show(notification_template, [](const rainy::notification_event &e) {
        if (e.type == rainy::notification_event::event_type::activated) {
            std::wcout << L"Activated!" << std::endl;
        } else if (e.type == rainy::notification_event::event_type::activated_with_reply) {
            std::wcout << L"Activated with reply: " << std::get<std::wstring_view>(e.data) << "\n";
        }
    });
	system("pause");
	return 0;
}
```

### Note

The open-source license for this library differs from that of WinToast. This library is distributed under the Apache 2.0 license, rather than MIT. Please take note of this. The source code is entirely rewritten using WinRT, which is why it uses a different license.

How to Use
This library already provides annotated documentation, but it's in Chinese. However, from the function names, you can generally infer their purpose. Additionally, all names, except for templates, follow the snake_case naming convention.

The following is just a demo, but it also demonstrates some basic features.

Updates will continue to be made. I’ll try my best to maintain it.
