#!/bin/bash

TIME=$(date +%s)
chmod 600 .travis/id_rsa
mkdir OpenTechEngine-${TIME}
cp -a base OpenTechEngine-${TIME}/
cp build/bin/OpenTechEngine.exe OpenTechEngine-${TIME}/;
find build -name \*dll -exec cp '{}' OpenTechEngine-${TIME}/ \; 
cp libs/openal/openal-soft/bin/Win64/soft_oal.dll OpenTechEngine-${TIME}/OpenAL32.dll 
cp /usr/lib/gcc/x86_64-w64-mingw32/4.8/libgcc_s_sjlj-1.dll /usr/lib/gcc/x86_64-w64-mingw32/4.8/libstdc++-6.dll /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll OpenTechEngine-${TIME}/
zip -r OpenTechEngine-${TIME}.zip OpenTechEngine-${TIME}/
scp -B -i .travis/id_rsa -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no OpenTechEngine-${TIME}.zip kordex@frs.sourceforge.net:/home/frs/project/opentechengine/snapshot/Windows/MinGW/OpenTechEngine-${TIME}.zip

