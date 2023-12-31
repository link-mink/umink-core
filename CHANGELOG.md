## [v1.0.2](https://github.com/link-mink/umink-core/compare/v1.0.1..v1.0.2) - 2023-10-06
#### Bug Fixes
- **(tests)** define src and tests sonar dirs - ([f48cbb9](https://github.com/link-mink/umink-core/commit/f48cbb93a624352ecf4efb19c1d407f5fc6ac6ba)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** ignore duplications in unit tests - ([d3888b6](https://github.com/link-mink/umink-core/commit/d3888b6b80885edd5f2c4cd591dbef0fc5ae2fd7)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** do not ignore umink_plugin - ([d81edf6](https://github.com/link-mink/umink-core/commit/d81edf600118bd4a484bcd4dc898fd00b4c89f8b)) - [@dfranusic](https://github.com/dfranusic)
- **(umlua)** fix shutdown SIGSEGV - ([41658fe](https://github.com/link-mink/umink-core/commit/41658fe4a157cac6f5857b11a4def7074e9da6a1)) - [@dfranusic](https://github.com/dfranusic)
#### Features
- **(tests)** add umlua sub-module test - ([ad33671](https://github.com/link-mink/umink-core/commit/ad33671fe1cd6e39a6153bb87b81e51582b1ffca)) - [@dfranusic](https://github.com/dfranusic)
#### Miscellaneous Chores
- **(version)** v1.0.2 - ([4627a42](https://github.com/link-mink/umink-core/commit/4627a425fe3f18d951e65a9ec726453242d3a210)) - [@dfranusic](https://github.com/dfranusic)

- - -

## [v1.0.1](https://github.com/link-mink/umink-core/compare/v1.0.0..v1.0.1) - 2023-10-06
#### Bug Fixes
- **(build)** update github workflows - ([9e7a74e](https://github.com/link-mink/umink-core/commit/9e7a74e239949d42fe418f6b1a624121623e51d5)) - [@dfranusic](https://github.com/dfranusic)
- **(build)** update cog configuration - ([822e8d6](https://github.com/link-mink/umink-core/commit/822e8d615bbf3ae5c70da8fe5bcb2dc58c5d66c3)) - [@dfranusic](https://github.com/dfranusic)
- **(build)** use light tags in git version script - ([851d56e](https://github.com/link-mink/umink-core/commit/851d56eb705ef294b0e9d6cfbb1bf1b49bca6614)) - [@dfranusic](https://github.com/dfranusic)
- **(build)** fix missing mosquitto_broker include - ([eed6445](https://github.com/link-mink/umink-core/commit/eed6445ed7ba0af0ddd7ef37e805cfdf22403f54)) - [@dfranusic](https://github.com/dfranusic)
- **(build)** add missing umcounters header to dist - ([d6c58f1](https://github.com/link-mink/umink-core/commit/d6c58f1dbfb16f344ed06ef948cfa2db894613c4)) - [@dfranusic](https://github.com/dfranusic)
- **(build)** add .version to .gitignore - ([a8e02d9](https://github.com/link-mink/umink-core/commit/a8e02d9c6cfbbaa92d8331b49950cc7db21f0460)) - [@dfranusic](https://github.com/dfranusic)
- **(mqtt)** set ON_CONNECT callback earlier - ([2531d5b](https://github.com/link-mink/umink-core/commit/2531d5b416a13ead8958fbdbcf38f666edae45ca)) - [@dfranusic](https://github.com/dfranusic)
- **(mqtt)** fix "bin_upload_path" segfault - ([98b1b49](https://github.com/link-mink/umink-core/commit/98b1b4900080eebf8cbca3c27b6557dcfc2cd1c5)) - [@dfranusic](https://github.com/dfranusic)
- **(mqtt)** fix connection's "invalid object" issue - ([c4ccec7](https://github.com/link-mink/umink-core/commit/c4ccec7e2b088ca7e1f7b511e84d8c183024dc8d)) - [@dfranusic](https://github.com/dfranusic)
- **(mqtt)** add another fix for paho deadlocks - ([2199544](https://github.com/link-mink/umink-core/commit/2199544e68929726a7746f9f794c7775b1f8dab0)) - [@dfranusic](https://github.com/dfranusic)
- **(mqtt)** fix paho MQTTAsync_assignMsgId deadlock - ([7a5dc9b](https://github.com/link-mink/umink-core/commit/7a5dc9bb8a96b1cd7549d674a1bc7a1514cfe938)) - [@dfranusic](https://github.com/dfranusic)
- **(mqtt)** fix RX handler memory leak - ([37c692c](https://github.com/link-mink/umink-core/commit/37c692c70c592d639ca79fda587ba6ebdb42adfd)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** remove inspect module dependency - ([d16c7a8](https://github.com/link-mink/umink-core/commit/d16c7a8aeea4a258041c70cf6d8df670adf84527)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** use address sanitizer for memory leaks - ([97476f9](https://github.com/link-mink/umink-core/commit/97476f916928781a820d51f513c430adcb86f7ae)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** add plugin invalid arg count test - ([09a19c3](https://github.com/link-mink/umink-core/commit/09a19c34dbf9c417408d08a55ffa367d819c7c29)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** use init/dtor in some tests - ([420d0d1](https://github.com/link-mink/umink-core/commit/420d0d14a69f88fabf04c75028d27415466d4d06)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** fix cmocka's strdup memory tracking - ([2796c44](https://github.com/link-mink/umink-core/commit/2796c44fcc5664e765c53746358336b80ac93603)) - [@dfranusic](https://github.com/dfranusic)
- **(umlua)** allow SIGNAL handler to return "" - ([919dd4d](https://github.com/link-mink/umink-core/commit/919dd4d9da7f6ab1fa5149f5e41d37cfe39f80cd)) - [@dfranusic](https://github.com/dfranusic)
- do not use RTLD_GLOBAL for plugins - ([63acfe5](https://github.com/link-mink/umink-core/commit/63acfe56bd9312794a1e8bc760b1cacc4372a9ef)) - [@dfranusic](https://github.com/dfranusic)
- do not use FNM_EXTMATCH with musl - ([45d296e](https://github.com/link-mink/umink-core/commit/45d296e23a274ed83de32d06532108e74ba52c82)) - [@dfranusic](https://github.com/dfranusic)
#### Build system
- update .gitignore - ([fab36d6](https://github.com/link-mink/umink-core/commit/fab36d6159a14df7222e5c9e4a918a6e17b02c3c)) - [@dfranusic](https://github.com/dfranusic)
- add cocogitto configuration - ([94a5492](https://github.com/link-mink/umink-core/commit/94a54927aba84dd39e350163a901f7a5580a96a4)) - [@dfranusic](https://github.com/dfranusic)
- change clang-format - ([82848a5](https://github.com/link-mink/umink-core/commit/82848a5cd02fc5bb9612ed7a4a64c10b9efba72e)) - [@dfranusic](https://github.com/dfranusic)
- add git version to autoconf - ([b011798](https://github.com/link-mink/umink-core/commit/b0117987ed75353fd3de1904b71ac1c56a11b78a)) - [@dfranusic](https://github.com/dfranusic)
- add autogen script - ([7d81156](https://github.com/link-mink/umink-core/commit/7d81156484fbe312e53ba8d18f3f31b2806a0c71)) - [@dfranusic](https://github.com/dfranusic)
#### Features
- **(build)** add coverage data for sonarcloud - ([1db2fdd](https://github.com/link-mink/umink-core/commit/1db2fdd9e9c78a9e65350fc0cf206bd50a41faf8)) - [@dfranusic](https://github.com/dfranusic)
- **(build)** setup sonarcloud - ([4597e23](https://github.com/link-mink/umink-core/commit/4597e23cb9753ede80d483adb4055a57426d8a98)) - [@dfranusic](https://github.com/dfranusic)
- **(build)** add github build workflow - ([67e5fdd](https://github.com/link-mink/umink-core/commit/67e5fdd2bb34de6fd0e44ae7817b59c1442f9df8)) - [@dfranusic](https://github.com/dfranusic)
- **(build)** update .gitignore - ([9bb7f6a](https://github.com/link-mink/umink-core/commit/9bb7f6a2b4b18c21aee6ea40ae6949fbdf61aa64)) - [@dfranusic](https://github.com/dfranusic)
- **(lua)** remove Lua plugin - ([6b8ee44](https://github.com/link-mink/umink-core/commit/6b8ee4433ce660dae5c28e49bc17fed732158d0b)) - [@dfranusic](https://github.com/dfranusic)
- **(lua)** use per-thread lua_state - ([4170e4c](https://github.com/link-mink/umink-core/commit/4170e4c5aacb268618401bede8e75db25c1e210d)) - [@dfranusic](https://github.com/dfranusic)
- **(lua)** add DB set/get for custom user data - ([a5277e2](https://github.com/link-mink/umink-core/commit/a5277e2efb1b67d38c58d98ffc0499b0c124ff90)) - [@dfranusic](https://github.com/dfranusic)
- **(mqtt)** expose MQTT as a Lua sub-module - ([6c77264](https://github.com/link-mink/umink-core/commit/6c77264ca183847d33254ae3eca3a46bd86ea872)) - [@dfranusic](https://github.com/dfranusic)
- **(mqtt)** add error handling for binary upload - ([03772d9](https://github.com/link-mink/umink-core/commit/03772d9cfd60eb8f3d8e027b9ffc0f5e375ca93f)) - [@dfranusic](https://github.com/dfranusic)
- **(mqtt)** add support for MQTT binary file upload - ([2587c22](https://github.com/link-mink/umink-core/commit/2587c22c913f4a1cde9f942e82791bd0c146c708)) - [@dfranusic](https://github.com/dfranusic)
- **(mqtt)** add "keep_alive" connection parameter - ([088bd13](https://github.com/link-mink/umink-core/commit/088bd1399b5d86a055b43dc859c36e72c92748ed)) - [@dfranusic](https://github.com/dfranusic)
- **(test)** optimize and add user role auth tests - ([99c9494](https://github.com/link-mink/umink-core/commit/99c9494c8c1f91afab0e36a3632a7b06014ee7c3)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** update umd tests - ([42141da](https://github.com/link-mink/umink-core/commit/42141dae9be4481ecfe156117e494a80891eedfa)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** add umlua tests for cmd_call - ([68889a9](https://github.com/link-mink/umink-core/commit/68889a90e1e1dcbd3baa7c40fb9699ecd58db44a)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** add umlua tests for special cases - ([d5a58a1](https://github.com/link-mink/umink-core/commit/d5a58a150890d398c14cb33fd75a58cf02fa29d6)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** check lua env and umcounters via M - ([4c6d850](https://github.com/link-mink/umink-core/commit/4c6d85060edcaf61e5f1a736499c076f39b998b0)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** add initial umlua tests - ([9120a89](https://github.com/link-mink/umink-core/commit/9120a89df32dc0d32c6f0c667b6fcec54b1a72ec)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** add unit tests for plugin arguments - ([82ced95](https://github.com/link-mink/umink-core/commit/82ced95e6fcd553228da8bd795069471c1b53e5e)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** add unit tests for loading um plugins - ([fc21dbf](https://github.com/link-mink/umink-core/commit/fc21dbfa08daeb39c784151a535ce726a3ec6924)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** add unit tests for umdaemon - ([049f64c](https://github.com/link-mink/umink-core/commit/049f64c6046b3e053712911300779ab43e8d86ef)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** add unit tests for umcounters - ([fda7f00](https://github.com/link-mink/umink-core/commit/fda7f005ff94015eeb2c0028c5d9eeef88369f09)) - [@dfranusic](https://github.com/dfranusic)
- **(tests)** add unit tests for umdb methods - ([a7b1201](https://github.com/link-mink/umink-core/commit/a7b120110e2200fbc3a3a237da907c1102a85a71)) - [@dfranusic](https://github.com/dfranusic)
- **(umlua)** fix realloc memory issue for events - ([58041a4](https://github.com/link-mink/umink-core/commit/58041a45b306f81e772fed6276bee80beaef2266)) - [@dfranusic](https://github.com/dfranusic)
- fix sonarcloud reported issues - ([7ff0e66](https://github.com/link-mink/umink-core/commit/7ff0e66623a5372c2205bce1386a90589be42838)) - [@dfranusic](https://github.com/dfranusic)
- add umplg signal matching w/fnmatch - ([1be6ffc](https://github.com/link-mink/umink-core/commit/1be6ffc98fc2e9f77c1e913de81470624f66eba4)) - [@dfranusic](https://github.com/dfranusic)
- add SIGNAL handler authentication level - ([d73b4ed](https://github.com/link-mink/umink-core/commit/d73b4ed90c81bddf64e58304e87aedc9a5457ea4)) - [@dfranusic](https://github.com/dfranusic)
- introduce a two-phase plugin termination - ([16f2cad](https://github.com/link-mink/umink-core/commit/16f2cad2b03828c01b50550548c21fddffc419ed)) - [@dfranusic](https://github.com/dfranusic)
- add mosquitto v5 plugin - ([60b2b5b](https://github.com/link-mink/umink-core/commit/60b2b5bf955500950d1cc04bfdc85c653a749d2b)) - [@dfranusic](https://github.com/dfranusic)
- create global perf context - ([18aa908](https://github.com/link-mink/umink-core/commit/18aa908e213438c72c082a2921463c79d60089d2)) - [@dfranusic](https://github.com/dfranusic)
- update header documentation - ([9169862](https://github.com/link-mink/umink-core/commit/91698628396edca17caa33abc1ee49414a6e5ee9)) - [@dfranusic](https://github.com/dfranusic)
- Rename umcounter to umc - ([9524347](https://github.com/link-mink/umink-core/commit/9524347f023b40a30a1482d509a8b399d1d9d062)) - [@dfranusic](https://github.com/dfranusic)
- Make COAP RPC optional - ([3a69382](https://github.com/link-mink/umink-core/commit/3a693825d147243bbc895d8a8be60fc3bb826730)) - [@dfranusic](https://github.com/dfranusic)
- Add COAP and perf counters - ([6f13fac](https://github.com/link-mink/umink-core/commit/6f13faca68b9a862bbfa201475595e6270661eac)) - [@dfranusic](https://github.com/dfranusic)
- Add extra CAS op in atomics - ([3a0f438](https://github.com/link-mink/umink-core/commit/3a0f438f0bebbeaffbcace904d18229ac63d6f74)) - [@dfranusic](https://github.com/dfranusic)
#### Miscellaneous Chores
- **(version)** v1.0.1 - ([8e6de26](https://github.com/link-mink/umink-core/commit/8e6de26ad196bcc2f968c06484adc9d3727c48bd)) - [@dfranusic](https://github.com/dfranusic)
- add gcov data to gitignore - ([4a6d127](https://github.com/link-mink/umink-core/commit/4a6d1274fc639fdb35831d0a4898132ba1937013)) - [@dfranusic](https://github.com/dfranusic)
- add sonarcloud badge - ([f6795d6](https://github.com/link-mink/umink-core/commit/f6795d63d4e6d43fa118045e2953ed28d23afe09)) - [@dfranusic](https://github.com/dfranusic)
- update README - ([ea12aa5](https://github.com/link-mink/umink-core/commit/ea12aa5fbe470b2875b07ab4d7e8d381f7a26197)) - [@dfranusic](https://github.com/dfranusic)
- update gitignore - ([1d4dfe6](https://github.com/link-mink/umink-core/commit/1d4dfe62221c5a45a272433f876fc0de493b3530)) - [@dfranusic](https://github.com/dfranusic)

- - -

## [v1.0.0](https://github.com/link-mink/umink-core/compare/09fc3796a6b10118c04f52f82a2658d4810cf0b5..v1.0.0) - 2023-10-06
#### Bug Fixes
- **(lua)** Do not register a faulty signal handler - ([8365e03](https://github.com/link-mink/umink-core/commit/8365e032f19bfead2cdb3aff5d98a2963b604cee)) - [@dfranusic](https://github.com/dfranusic)
- **(lua)** allocate output buffer in SIG handler - ([02c6022](https://github.com/link-mink/umink-core/commit/02c602294aacffe1f127675271ec32ebaf127e8d)) - [@dfranusic](https://github.com/dfranusic)
- **(mqtt)** Add missing MQTT payload in Lua signal - ([a91a2e6](https://github.com/link-mink/umink-core/commit/a91a2e651e43c0b65c5657b2c89218820b52c88e)) - [@dfranusic](https://github.com/dfranusic)
- remove mosquitto_broker include - ([30db795](https://github.com/link-mink/umink-core/commit/30db795a8741d06e263750a474c0dd7d5ccc818e)) - [@dfranusic](https://github.com/dfranusic)
- add missing macros for Lua 5.1 - ([89dac07](https://github.com/link-mink/umink-core/commit/89dac07c42a9570a851be0136307f6512a4f0aec)) - [@dfranusic](https://github.com/dfranusic)
- change invalid mqtt topic pointer - ([721df8c](https://github.com/link-mink/umink-core/commit/721df8cde34a8f72563a5aa5546f1e78b3696f3c)) - [@dfranusic](https://github.com/dfranusic)
- modify mqtt configuration methods - ([76d662d](https://github.com/link-mink/umink-core/commit/76d662d3ef069a698b749c8a318311d49c6990b8)) - [@dfranusic](https://github.com/dfranusic)
#### Build system
- install mink lua module - ([cd720a0](https://github.com/link-mink/umink-core/commit/cd720a077d3e14e6c23fc0528de25c4048b39fc2)) - [@dfranusic](https://github.com/dfranusic)
#### Features
- **(build)** update gitignore - ([146b1ae](https://github.com/link-mink/umink-core/commit/146b1aea1a4450aa44af82cbeaa90cdba1bb2a4c)) - [@dfranusic](https://github.com/dfranusic)
- **(build)** setup autotools - ([fa2bb54](https://github.com/link-mink/umink-core/commit/fa2bb5469618737e26365cdd3a5484162201501a)) - [@dfranusic](https://github.com/dfranusic)
- **(lua)** Improve Lua logging - ([6dd503b](https://github.com/link-mink/umink-core/commit/6dd503b7e567a8fc1cb3956b77617384249b52f7)) - [@dfranusic](https://github.com/dfranusic)
- add Lua signal recursion prevention - ([2a8a180](https://github.com/link-mink/umink-core/commit/2a8a18008a55f72ab1ab58ac76acf78dec39d99e)) - [@dfranusic](https://github.com/dfranusic)
- add support for Lua 5.x/LuaJIT(w/o ffi) - ([55f6cc5](https://github.com/link-mink/umink-core/commit/55f6cc5f0c6b2e0505991467e453513d076d9fed)) - [@dfranusic](https://github.com/dfranusic)
- add mosquitto auth plugin - ([9c911c7](https://github.com/link-mink/umink-core/commit/9c911c724a73fbcfdf616cfd59827c4ec69534f8)) - [@dfranusic](https://github.com/dfranusic)
- bundle uthash - ([d7872ec](https://github.com/link-mink/umink-core/commit/d7872ec2963fe3b4ea1a0c4c0c7e8269edabca06)) - [@dfranusic](https://github.com/dfranusic)
- add initial README file - ([0cc47de](https://github.com/link-mink/umink-core/commit/0cc47de010b1acaae75a10450208cc08739f2d5a)) - [@dfranusic](https://github.com/dfranusic)
- implement graceful shutdown for lua and mqtt - ([920d1c5](https://github.com/link-mink/umink-core/commit/920d1c524dcd99f792b63cde72a6ff563e9bd072)) - [@dfranusic](https://github.com/dfranusic)
- implement plg2plg CMD_MQTT_PUBLISH method - ([bc3fcd4](https://github.com/link-mink/umink-core/commit/bc3fcd4b4ef2f181547ccd0d35bf7263a332dc85)) - [@dfranusic](https://github.com/dfranusic)
- apply the new clang-format - ([52e7630](https://github.com/link-mink/umink-core/commit/52e76304aa4c3b18d2c61a2eaa55ff28b7f2dd4d)) - [@dfranusic](https://github.com/dfranusic)
- update clang-format configuration - ([02d580f](https://github.com/link-mink/umink-core/commit/02d580fecd68f75581252a676e61bc531b21d6e5)) - [@dfranusic](https://github.com/dfranusic)
- add initial mqtt plugin - ([fdbf9fb](https://github.com/link-mink/umink-core/commit/fdbf9fb9e5b4927f394d34170be45aa8bfe8f7b1)) - [@dfranusic](https://github.com/dfranusic)
- add Lua signals - ([dd747f2](https://github.com/link-mink/umink-core/commit/dd747f21cfff73c65ffbd2916b153b9162db8e35)) - [@dfranusic](https://github.com/dfranusic)
- add lua env thread - ([71eb65c](https://github.com/link-mink/umink-core/commit/71eb65cf76edea7d237824654e5943e13c1011de)) - [@dfranusic](https://github.com/dfranusic)
- add Lua env manager - ([d66a1b3](https://github.com/link-mink/umink-core/commit/d66a1b360752f39c5ed66c7a484af9993ea9a825)) - [@dfranusic](https://github.com/dfranusic)
- add Lua plugin draft - ([2b34db6](https://github.com/link-mink/umink-core/commit/2b34db6aafe100182a59634aca2be52ed24255eb)) - [@dfranusic](https://github.com/dfranusic)
- add plugin manager - ([52fb9bf](https://github.com/link-mink/umink-core/commit/52fb9bfc6620135b2b3babbefdce64d4a40b74b8)) - [@dfranusic](https://github.com/dfranusic)


