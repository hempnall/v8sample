g++ -I/root/v8/  main.cpp v8alg.cpp -o hello_world -Wl,--start-group ~/v8/out/native/obj.target/{tools/gyp/libv8_{base,libbase,snapshot,libplatform},third_party/icu/libicu{uc,i18n,data}}.a -Wl,--end-group -lrt -pthread
