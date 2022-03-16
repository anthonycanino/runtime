mkdir tmp
copy .\artifacts\packages\Debug\Shipping\dotnet-targeting-pack-7.0.0-dev-win-x64.zip tmp\
cd tmp
tar -xf dotnet-targeting-pack-7.0.0-dev-win-x64.zip
copy .\packs\Microsoft.NETCore.App.Ref\7.0.0-dev\ref\net7.0\System.Runtime.Intrinsics.* C:\Users\acanino\Dev\dotnet\runtime-7\packs\Microsoft.NETCore.App.Ref\7.0.0-preview.1.22076.8\ref\net7.0\
