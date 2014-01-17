@echo off
title 科大讯飞语音云SDK Demo配置程序
color A
echo ***********************************使用说明***********************************
echo *                                                                            *
echo *        本脚本用于vs2005/vs2008用户更改SDK中的Examples工程配置项，vs2010    *
echo *    或vs2012用户，不必运行本脚本。                                          *
echo *                                                                            *
echo *        由于工程配置项中，关于调试的部分是保存在以用户名和计算机名为组成    *
echo *    部分的文件中， SDK本身无法自动适配，所以通过这个脚本来进行配置，以简    *
echo *    化用户在调试Examples时配置环境的步骤。                                  *
echo *                                                                            *
echo *        如果您在调试Examples程序时，出现类似“找不到msc.dll ”的错误时，    *
echo *    请尝试关闭vs之后，运行本脚本来解决。                                    *
echo *                                                                            *
echo *        本脚本运行过程中收集到的所有信息都不会上传，没有泄露隐私的风险，    *
echo *    您可以自行查看源码来证明。                                              *
echo *                                                                            *
echo *        如果不希望使用，请点击右上角X退出。                                 *
echo *                                                                            *
echo ******************************************************************************
echo.
pause
echo.
echo 计算机名：%COMPUTERNAME%
echo 用户名  ：%USERNAME%
echo.
cd examples
for /d %%i in (*) do ( echo %%i
cd %%i
copy %%i.vcproj.bak %%i.vcproj.%COMPUTERNAME%.%USERNAME%.user 
cd .. )
cd ..
echo.
echo 执行完毕！
echo.
pause