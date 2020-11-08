..\tools\shadercRelease.exe -f .\vs_ibl_mesh.sc		-o .\vs_ibl_mesh.bin	-i .\ --type v --platform windows -p vs_5_0 --verbose --debug -O 0
..\tools\shadercRelease.exe -f .\vs_ibl_skybox.sc	-o .\vs_ibl_skybox.bin	-i .\ --type v --platform windows -p vs_5_0 --verbose --debug -O 0
..\tools\shadercRelease.exe -f .\fs_ibl_mesh.sc		-o .\fs_ibl_mesh.bin	-i .\ --type f --platform windows -p ps_5_0 --verbose --debug -O 0
..\tools\shadercRelease.exe -f .\fs_ibl_skybox.sc	-o .\fs_ibl_skybox.bin	-i .\ --type f --platform windows -p ps_5_0 --verbose --debug -O 0