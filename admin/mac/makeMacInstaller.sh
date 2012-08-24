#!/bin/bash
python ../admin/mac/macdeploy.py src/UDJ.app
cp ../admin/mac/qt.conf src/UDJ.app/Contents/Resources/
mv src/UDJ.app/Contents/plugins src/UDJ.app/Contents/Frameworks
#cp -R /Library/Frameworks/Sparkle.framework Contents/Frameworks
../admin/mac/create-dmg.sh src/UDJ.app
