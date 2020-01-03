#pragma once
#include <iostream>
using namespace std;
void getLoginInfo(char buf[32]) {
	char usrName[16] = {'\0'}, pwd[16] = { '\0' };
	cout << "请输入用户名:";
	cin >> usrName;
	cout << "请输入密码:";
	cin >> pwd;
	memcpy(buf, usrName, 16 * sizeof(char));
	memcpy(buf + 16 * sizeof(char), pwd, 16 * sizeof(char));
}