#!/bin/bash

chmod 600 .travis/id_rsa
find build/ > build/build.files.txt
find build/ -type f -iname \*rsp -o -iname \*log -o -iname \*txt -exec tar rf OpenTechFailure.tar '{}' \;
bzip2 -9 OpenTechFailure.tar
scp -B -i .travis/id_rsa -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no OpenTechFailure.tar.bz2 kordex@frs.sourceforge.net:/home/frs/project/testrepositoryproject/TestRel/$(date +%s).failed_build.$CXX.tar.bz2

