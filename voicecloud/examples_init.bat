@echo off
title �ƴ�Ѷ��������SDK Demo���ó���
color A
echo ***********************************ʹ��˵��***********************************
echo *                                                                            *
echo *        ���ű�����vs2005/vs2008�û�����SDK�е�Examples���������vs2010    *
echo *    ��vs2012�û����������б��ű���                                          *
echo *                                                                            *
echo *        ���ڹ����������У����ڵ��ԵĲ����Ǳ��������û����ͼ������Ϊ���    *
echo *    ���ֵ��ļ��У� SDK�����޷��Զ����䣬����ͨ������ű����������ã��Լ�    *
echo *    ���û��ڵ���Examplesʱ���û����Ĳ��衣                                  *
echo *                                                                            *
echo *        ������ڵ���Examples����ʱ���������ơ��Ҳ���msc.dll ���Ĵ���ʱ��    *
echo *    �볢�Թر�vs֮�����б��ű��������                                    *
echo *                                                                            *
echo *        ���ű����й������ռ�����������Ϣ�������ϴ���û��й¶��˽�ķ��գ�    *
echo *    ���������в鿴Դ����֤����                                              *
echo *                                                                            *
echo *        �����ϣ��ʹ�ã��������Ͻ�X�˳���                                 *
echo *                                                                            *
echo ******************************************************************************
echo.
pause
echo.
echo ���������%COMPUTERNAME%
echo �û���  ��%USERNAME%
echo.
cd examples
for /d %%i in (*) do ( echo %%i
cd %%i
copy %%i.vcproj.bak %%i.vcproj.%COMPUTERNAME%.%USERNAME%.user 
cd .. )
cd ..
echo.
echo ִ����ϣ�
echo.
pause