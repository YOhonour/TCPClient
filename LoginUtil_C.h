#pragma once
#include <iostream>
using namespace std;
void getLoginInfo(char buf[32]) {
	char usrName[16] = {'\0'}, pwd[16] = { '\0' };
	cout << "�������û���:";
	cin >> usrName;
	cout << "����������:";
	cin >> pwd;
	memcpy(buf, usrName, 16 * sizeof(char));
	memcpy(buf + 16 * sizeof(char), pwd, 16 * sizeof(char));
}