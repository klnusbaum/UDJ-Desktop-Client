#!/bin/bash
# Copyright 2012 Kurtis L. Nusbaum
#
# This file is part of UDJ.
#
# UDJ is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# UDJ is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with UDJ.  If not, see <http://www.gnu.org/licenses/>.

python ../admin/mac/macdeploy.py src/UDJ.app
cp ../admin/mac/qt.conf src/UDJ.app/Contents/Resources/
mv src/UDJ.app/Contents/plugins src/UDJ.app/Contents/Frameworks
#cp -R /Library/Frameworks/Sparkle.framework Contents/Frameworks
../admin/mac/create-dmg.sh src/UDJ.app
