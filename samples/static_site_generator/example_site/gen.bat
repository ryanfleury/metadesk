@echo off
if not exist generated mkdir generated
pushd generated
..\..\..\build\static_site_generator.exe --siteinfo ..\site_info.mdesk --pagedir ..\
popd