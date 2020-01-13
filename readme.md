# 3g-rebooter

Description(En-Us):
huawei-e173 3g-modem: reboot(shutdown) system by sms-message
Описание(Ru-Ru):
huawei-e173 3g-модем: reboot(shutdown) системы посредством sms-сообшения

Публичный SMS-шлюз для бесплатной отправки (ОПСПС Мегафон, Россия):
https://moscow.megafon.ru/help/info/message/

Формат SMS для перезагрузки ПК:
sys-r|s [timeout] [...]
где:
    r|s     - выполняемая команда reboot|shutdown,
    timeout - таймант (в секундах) для ожидания завершения работы и возможности отмены орерации
	[...]   - прочие полезные или дополнительные user-data (так, указанный выше sms-шлюз добавляет сюда суффиксные данные)

Идентификация отправителя (в целях безопасности) в настоящее время не производться, хотя отправитель программно определяется.
Достаточным считается знание формата сообщения; если он некорректен, то sms игнорируется.

История версий:
2020-01-08: версия 1.0

При старте необходимо принудительно завершать процессы:
1) "C:\ProgramData\Mobile Partner\OnlineUpdate\ouc.exe", так называемый "Online Update Client", x32, просто не нужен.
Подписан:
	CN = Huawei Technologies Co., Ltd.
	OU = Digital ID Class 3 - Microsoft Software Validation v2
	O = Huawei Technologies Co., Ltd.
	L = Shenzhen
	S = Guangdong
	C = CN
2) "C:\Program Files (x86)\Mobile Partner\Mobile Partner.exe", x32, занимает нужный com-порт в эксклюзиве.
Не подписан. Открытие com-порта фейлится с win32-ошибкой:
	$err,hr		ERROR_BUSY: "Требуемый ресурс занят."
