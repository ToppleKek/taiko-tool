#!/bin/bash
set -e
CC=gcc
SCRIPTPATH=$(dirname "$(readlink -f "$0")")

printf "This will install libg719_decode to /usr/lib/\n"

sudo -v
# Keep-alive: update existing sudo time stamp if set, otherwise do nothing.
while true; do sudo -n true; sleep 60; kill -0 "$$" || exit; done 2>/dev/null &

rm -rf /tmp/taiko_tool
mkdir -p /tmp/taiko_tool/tools
cd /tmp/taiko_tool

# Download and build libg719_decode
git clone https://github.com/kode54/libg719_decode libg719_decode
cd libg719_decode

C_FILES=$(find ./ -type f -name '*.c')
CFLAGS='-Wall -I./reference_code/include -Wno-comment -Wno-missing-braces -DVAR_ARRAYS -fPIC -shared'
LIBS='-lm'

$CC -o libg719_decode.so $CFLAGS $C_FILES $LIBS
sudo mv libg719_decode.so /usr/lib/

# Download and build vgmstream with G719 decoder enabled
cd ..
git clone https://github.com/ToppleKek/vgmstream vgmstream
cd vgmstream

./bootstrap

CFLAGS="-DVGM_USE_FFMPEG -DVGM_USE_G719" \
LIBS="-lavcodec -lavformat -lavutil -lg719_decode" \
./configure --prefix=/tmp/taiko_tool
make -f Makefile.autotools -j5
make -f Makefile.autotools install
mv /tmp/taiko_tool/bin/vgmstream-cli /tmp/taiko_tool/tools/vgmstream-cli
sudo mv /tmp/taiko_tool/lib/* /usr/lib/

mkdir -p /tmp/taiko_tool/dotnet
cd /tmp/taiko_tool/dotnet
wget -O dotnet.tar.gz https://download.visualstudio.microsoft.com/download/pr/e730fe40-e2ea-42e5-a5d0-f86830d75849/571e5a2f4ebf9f8117878eeaad5cb19b/dotnet-sdk-2.1.805-linux-x64.tar.gz
tar -xf dotnet.tar.gz
git clone https://github.com/Thealexbarney/VGAudio VGAudio
cd VGAudio/src/VGAudio.Cli
/tmp/taiko_tool/dotnet/dotnet new nuget
sed -i '/<add key=/d' nuget.config
sed -i '/<clear \/>/ a <add key="dotnet-core" value="https://dotnetfeed.blob.core.windows.net/dotnet-core/index.json" />\n<add key="nuget.org" value="https://api.nuget.org/v3/index.json" protocolVersion="3" />' nuget.config
/tmp/taiko_tool/dotnet/dotnet add package Microsoft.DotNet.ILCompiler -v 1.0.0-alpha-*
/tmp/taiko_tool/dotnet/dotnet publish -r linux-x64 -c Release --framework netcoreapp2.0
cp bin/Release/netcoreapp2.0/linux-x64/native/VGAudioCli /tmp/taiko_tool/tools

cp -r /tmp/taiko_tool/tools $SCRIPTPATH
rm -rf /tmp/taiko_tool

printf "\nDone! All needed tools are installed in ./tools\nYou should now set the paths to these tools in your Taiko Tool settings\n"
