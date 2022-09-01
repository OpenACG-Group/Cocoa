# literature/elegia: Known Bugs and Fixing

## 11 Feb 2022, Fri

* ### V8 v9.9 分支编译失败
生成配置：
```shell
gn gen out/release --args='use_custom_libcxx=false is_debug=false v8_monolithic=true v8_use_external_startup_data=false'
```

编译：
```shell
ninja -C out/release
```

此时，会报错：
```shell
FAILED: v8_cppgc_shared_unittests 
python3 "../../build/toolchain/gcc_link_wrapper.py" --output="./v8_cppgc_shared_unittests" -- ../../third_party/llvm-build/Release+Asserts/bin/clang++ -pie -fuse-ld=lld -Wl,--fatal-warnings -Wl,--build-id -fPIC -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -Wl,--icf=all -Wl,--color-diagnostics -Wl,--no-call-graph-profile-sort -m64 -no-canonical-prefixes -Werror -rdynamic --sysroot=../../build/linux/debian_sid_amd64-sysroot -Wl,-z,defs -Wl,--as-needed -pie -Wl,--disable-new-dtags -Wl,-O2 -Wl,--gc-sections -o "./v8_cppgc_shared_unittests" -Wl,--start-group @"./v8_cppgc_shared_unittests.rsp"  -Wl,--end-group  -latomic -ldl -lpthread -lrt
ld.lld: error: undefined symbol: testing::AssertionSuccess()
>>> referenced by worklist-unittest.cc
>>>               obj/test/unittests/v8_cppgc_shared_unittests_sources/worklist-unittest.o:(heap::base::WorkListTest_MultipleSegmentsStolen_Test::TestBody())
>>> referenced by worklist-unittest.cc
>>>               obj/test/unittests/v8_cppgc_shared_unittests_sources/worklist-unittest.o:(testing::AssertionResult testing::internal::CmpHelperEQ<unsigned int, unsigned long>(char const*, char const*, unsigned int const&, unsigned long const&))
>>> referenced by worklist-unittest.cc
>>>               obj/test/unittests/v8_cppgc_shared_unittests_sources/worklist-unittest.o:(testing::AssertionResult testing::internal::CmpHelperEQ<heap::base::SomeObject*, heap::base::SomeObject*>(char const*, char const*, heap::base::SomeObject* const&, heap::base::SomeObject* const&))
>>> referenced 14 more times
...
```
分析 ninja 报告的命令，在链接阶段找不到符号。ninja 通过 `build/toolchain/gcc_link_wrapper.py` 调用了 `clang++`
进行链接，需要链接的文件从 `v8_cppgc_shared_unittests.rsp` 读取。
单从链接器的输出分析，结合一些 Google，发现找不到的符号应该都来自于 `googletest` 库。
再进一步看 `v8_cppgc_shared_unittests.rsp` 的内容，发现确实链接了 `googletest` 相关的库。
挨个用 `nm` 命令查看这些库和链接对象的符号，没有任何文件提供了上述缺失的符号。
再查看位于 `third_party/googletest/src/googletest/src` 下的 `googletest` 源文件，
发现链接器报告的所有缺失的符号，都在 `gtest-assertion-result.cc` 文件中定义。
明显，严重怀疑 V8 没有编译此文件。

检查 `googletest` 的编译配置文件，发现如下可疑部分：
```gn
# Do NOT depend on this directly. Use //testing/gtest instead.
# See README.chromium for details.
source_set("gtest") {
  testonly = true
  sources = [
    "src/googletest/include/gtest/gtest-death-test.h",
    "src/googletest/include/gtest/gtest-matchers.h",
    "src/googletest/include/gtest/gtest-message.h",
    "src/googletest/include/gtest/gtest-param-test.h",
    "src/googletest/include/gtest/gtest-printers.h",
    "src/googletest/include/gtest/gtest-spi.h",
    "src/googletest/include/gtest/gtest-test-part.h",
    "src/googletest/include/gtest/gtest-typed-test.h",
    "src/googletest/include/gtest/gtest.h",
    "src/googletest/include/gtest/gtest_pred_impl.h",
    "src/googletest/include/gtest/internal/gtest-death-test-internal.h",
    "src/googletest/include/gtest/internal/gtest-filepath.h",
    "src/googletest/include/gtest/internal/gtest-internal.h",
    "src/googletest/include/gtest/internal/gtest-param-util.h",
    "src/googletest/include/gtest/internal/gtest-port.h",
    "src/googletest/include/gtest/internal/gtest-string.h",
    "src/googletest/include/gtest/internal/gtest-type-util.h",

    #"src/googletest/src/gtest-all.cc",  # Not needed by our build.
    "src/googletest/src/gtest-death-test.cc",
    "src/googletest/src/gtest-filepath.cc",
    "src/googletest/src/gtest-internal-inl.h",
    "src/googletest/src/gtest-matchers.cc",
    "src/googletest/src/gtest-port.cc",
    "src/googletest/src/gtest-printers.cc",
    "src/googletest/src/gtest-test-part.cc",
    "src/googletest/src/gtest-typed-test.cc",
    "src/googletest/src/gtest.cc",
  ]
  ...
}
```
"Not needed by our build" 着实是一条很惹眼的注释。缺失的符号所在的 `gtest-assertion-result.cc`
文件恰好没有被包含到这个列表中。于是手动添加之，重新编译，无报错。问题解决。

至于为什么 Google V8 的 release 发布中会有如此明显的错误，原因暂且不明。
