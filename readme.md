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
2020-01-19: ������ 1.1
	application::close_some_processes()
2020-01-19: ������ 1.1.1
	fix pdu-ucs2 decoding
2020-01-23: ������ 1.1.2
	device::find(), smart-wait if com-port can't open with ERROR_BUSY win32-error code
2020-01-29: ������ 1.1.3
	close_some_processes(), fix 'importance_ex' lists traverse (not-tested)

��� ������ ���������� ������������� ��������� ��������:
1) "C:\ProgramData\Mobile Partner\OnlineUpdate\ouc.exe", 
	��� ���������� "Online Update Client", x32, ������ �� �����
����� ���� ��������, � ����� ���� � ���. ���� ��������, ��������:
	CN = Huawei Technologies Co., Ltd.
	OU = Digital ID Class 3 - Microsoft Software Validation v2
	O = Huawei Technologies Co., Ltd.
	L = Shenzhen
	S = Guangdong
	C = CN
2) "C:\Program Files (x86)\Mobile Partner\Mobile Partner.exe","C:\Program Files (x86)\MegaFon Internet\MegaFon Internet.exe", 
	x32, �������� ������ com-���� � ����������
�� ���������. ��� ���� �������� com-����� �������� � win32-�������:
	$err,hr		ERROR_BUSY: "��������� ������ �����."
