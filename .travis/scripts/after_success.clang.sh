#!/bin/bash

TIME=$(date +%s)
chmod 600 .travis/id_rsa
mkdir OpenTechEngine-${TIME}
cp -a base OpenTechEngine-${TIME}/
cp build/OpenTechEngine OpenTechEngine-${TIME}/
tar cjf OpenTechEngine-${TIME}.tar.bz2 OpenTechEngine-${TIME}/

scp -B -i .travis/id_rsa -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no OpenTechEngine-${TIME}.tar.bz2 kordex@frs.sourceforge.net:/home/frs/project/opentechengine/snapshot/Linux/Clang/OpenTechEngine-${TIME}.tar.bz2
