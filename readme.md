# 3g-rebooter

Description(En-Us):
huawei-e173 3g-modem: reboot(shutdown) system by sms-message
��������(Ru-Ru):
huawei-e173 3g-�����: reboot(shutdown) ������� ����������� sms-���������

��������� SMS-���� ��� ���������� �������� (����� �������, ������):
https://moscow.megafon.ru/help/info/message/

������ SMS ��� ������������ ��:
sys-r|s [timeout] [...]
���:
    r|s     - ����������� ������� reboot|shutdown,
    timeout - ������� (� ��������) ��� �������� ���������� ������ � ����������� ������ ��������
	[...]   - ������ �������� ��� �������������� user-data (���, ��������� ���� sms-���� ��������� ���� ���������� ������)

������������� ����������� (� ����� ������������) � ��������� ����� �� ������������, ���� ����������� ���������� ������������.
����������� ��������� ������ ������� ���������; ���� �� �����������, �� sms ������������.

������� ������:
2020-01-08: ������ 1.0

��� ������ ���������� ������������� ��������� ��������:
1) "C:\ProgramData\Mobile Partner\OnlineUpdate\ouc.exe", ��� ���������� "Online Update Client", x32, ������ �� �����.
��������:
	CN = Huawei Technologies Co., Ltd.
	OU = Digital ID Class 3 - Microsoft Software Validation v2
	O = Huawei Technologies Co., Ltd.
	L = Shenzhen
	S = Guangdong
	C = CN
2) "C:\Program Files (x86)\Mobile Partner\Mobile Partner.exe", x32, �������� ������ com-���� � ����������.
�� ��������. �������� com-����� �������� � win32-�������:
	$err,hr		ERROR_BUSY: "��������� ������ �����."
