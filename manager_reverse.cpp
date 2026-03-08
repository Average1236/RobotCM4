
/* zos::manager::manager(char const*, char const*) */

void __thiscall zos::manager::manager(manager *this,char *param_1,char *param_2)

{
  ContextManager *this_00;
  undefined4 uVar1;
  uint uVar2;
  io_context *piVar3;
  char *pcVar4;
  char *local_b0;
  manager *local_a8;
  string asStack_a0 [32];
  allocator aaStack_80 [8];
  allocator aaStack_78 [8];
  undefined1 auStack_70 [8];
  undefined8 local_68;
  undefined8 uStack_60;
  void *local_58;
  undefined **local_50;
  undefined1 auStack_48 [8];
  undefined8 local_40;
  undefined8 uStack_38;
  undefined **local_30;
  allocator aaStack_28 [8];
  allocator *local_20;
  allocator *local_18;
  allocator *local_10;
  undefined4 local_4;
  
  local_10 = aaStack_80;
  local_b0 = param_1;
  local_a8 = this;
                    /* try { // try from 0021dc7c to 0021dc7f has its CatchHandler @ 0021de84 */
  std::__cxx11::string::string<>((string *)this,param_1,aaStack_80);
  local_18 = aaStack_78;
                    /* try { // try from 0021dcac to 0021dcaf has its CatchHandler @ 0021de74 */
  std::__cxx11::string::string<>((string *)(local_a8 + 0x20),param_2,aaStack_78);
                    /* try { // try from 0021dcb4 to 0021dcd3 has its CatchHandler @ 0021de60 */
  uVar1 = getProcessId();
  *(undefined4 *)(local_a8 + 0x40) = uVar1;
  generateUUID[abi:cxx11]();
  std::thread::thread((thread *)(local_a8 + 0x68));
  std::atomic<bool>::atomic((atomic<bool> *)(local_a8 + 0x70),false);
  this_00 = (ContextManager *)(local_a8 + 0x78);
                    /* try { // try from 0021dcf8 to 0021dcfb has its CatchHandler @ 0021de40 */
  piVar3 = Singleton<>::_<>();
  udp::ContextManager::ContextManager(this_00,piVar3);
  InfoHandler::InfoHandler((InfoHandler *)(local_a8 + 0xc0));
  pcVar4 = getenv("ZOS_DEBUG_MANAGER");
  if (pcVar4 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_68,"inside manager address : {},name-{}\n");
                    /* try { // try from 0021dd50 to 0021de0b has its CatchHandler @ 0021de20 */
    local_58 = fmt::v9::ptr<>(local_a8);
    local_50 = &PTR_s_/home/pi/temp/zos_turing/project_002a8ad0;
    status<>::status(auStack_70,local_68,uStack_60,&local_58,&local_b0,&local_50);
  }
  uVar2 = std::__cxx11::string::empty((string *)local_a8);
  if ((uVar2 & 1) != 0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_40,
               "zos manager name is empty,maybe you need to set one name on ctor.\n");
    local_30 = &PTR_s_/home/pi/temp/zos_turing/project_002a8ae8;
    warning<>::warning(auStack_48,local_40,uStack_38,&local_30);
  }
  udp::ContextManager::start((ContextManager *)(local_a8 + 0x78));
  local_20 = aaStack_28;
  std::__cxx11::string::string<>(asStack_a0,"",aaStack_28);
  local_4 = 0;
  std::__cxx11::string::~string(asStack_a0);
  return;
}


/* zos::manager::~manager() */

void __thiscall zos::manager::~manager(manager *this)

{
  std::atomic<bool>::operator=((atomic<bool> *)(this + 0x70),true);
  udp::ContextManager::stop((ContextManager *)(this + 0x78));
  InfoHandler::~InfoHandler((InfoHandler *)(this + 0xc0));
  udp::ContextManager::~ContextManager((ContextManager *)(this + 0x78));
  std::thread::~thread((thread *)(this + 0x68));
  std::__cxx11::string::~string((string *)(this + 0x48));
  std::__cxx11::string::~string((string *)(this + 0x20));
  std::__cxx11::string::~string((string *)this);
  Singleton<>::~Singleton((Singleton<> *)this);
  return;
}


/* zos::manager::addPub(zos::PublisherBase*, zos::PublisherInfo) */

bool __thiscall zos::manager::addPub(manager *this,PublisherBase *param_1,long param_3)

{
  bool bVar1;
  char *pcVar2;
  HandleResult local_70 [8];
  undefined1 auStack_68 [32];
  undefined1 auStack_48 [8];
  undefined8 local_40;
  undefined8 uStack_38;
  void *local_30;
  undefined **local_28;
  undefined1 auStack_20 [8];
  undefined8 local_18;
  undefined8 uStack_10;
  undefined **local_8;
  
  pcVar2 = getenv("ZOS_DEBUG_MANAGER");
  if (pcVar2 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_40,"addPub address : {},key-{}\n");
    local_30 = fmt::v9::ptr<>(param_1);
    local_28 = &PTR_s_/home/pi/temp/zos_turing/project_002a8b00;
    status<>::status(auStack_48,local_40,uStack_38,&local_30,param_3 + 0x40,&local_28);
  }
  InfoHandler::addPub((PublisherBase *)(this + 0xc0),(PublisherInfo *)param_1);
  bVar1 = (((byte)local_70[0] ^ 1) & 1) == 0;
  if (!bVar1) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_18,"addPub failed : {}\n");
    local_8 = &PTR_s_/home/pi/temp/zos_turing/project_002a8b18;
                    /* try { // try from 0021e024 to 0021e027 has its CatchHandler @ 0021e044 */
    error<>::error(auStack_20,local_18,uStack_10,auStack_68,&local_8);
  }
  HandleResult::~HandleResult(local_70);
  return bVar1;
}


/* zos::manager::addSub(zos::SubscriberBase*, zos::SubscriberInfo) */

bool __thiscall zos::manager::addSub(manager *this,SubscriberBase *param_1,long param_3)

{
  bool bVar1;
  char *pcVar2;
  HandleResult local_70 [8];
  undefined1 auStack_68 [32];
  undefined1 auStack_48 [8];
  undefined8 local_40;
  undefined8 uStack_38;
  void *local_30;
  undefined **local_28;
  undefined1 auStack_20 [8];
  undefined8 local_18;
  undefined8 uStack_10;
  undefined **local_8;
  
  pcVar2 = getenv("ZOS_DEBUG_MANAGER");
  if (pcVar2 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_40,"addSub address : {},key-{}\n");
    local_30 = fmt::v9::ptr<>(param_1);
    local_28 = &PTR_s_/home/pi/temp/zos_turing/project_002a8b30;
    status<>::status(auStack_48,local_40,uStack_38,&local_30,param_3 + 0x48,&local_28);
  }
  InfoHandler::addSub((SubscriberBase *)(this + 0xc0),(SubscriberInfo *)param_1);
  bVar1 = (((byte)local_70[0] ^ 1) & 1) == 0;
  if (!bVar1) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_18,"addSub failed : {}\n");
    local_8 = &PTR_s_/home/pi/temp/zos_turing/project_002a8b48;
                    /* try { // try from 0021e164 to 0021e167 has its CatchHandler @ 0021e184 */
    error<>::error(auStack_20,local_18,uStack_10,auStack_68,&local_8);
  }
  HandleResult::~HandleResult(local_70);
  return bVar1;
}


/* zos::manager::removePub(zos::PublisherBase*, zos::PublisherInfo) */

bool zos::manager::removePub(long param_1,PublisherBase *param_2)

{
  bool bVar1;
  char *pcVar2;
  HandleResult local_70 [8];
  undefined1 auStack_68 [32];
  undefined1 auStack_48 [8];
  undefined8 local_40;
  undefined8 uStack_38;
  void *local_30;
  undefined **local_28;
  undefined1 auStack_20 [8];
  undefined8 local_18;
  undefined8 uStack_10;
  undefined **local_8;
  
  pcVar2 = getenv("ZOS_DEBUG_MANAGER");
  if (pcVar2 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_40,"removePub address : {}\n");
    local_30 = fmt::v9::ptr<>(param_2);
    local_28 = &PTR_s_/home/pi/temp/zos_turing/project_002a8b60;
    status<>::status(auStack_48,local_40,uStack_38,&local_30,&local_28);
  }
  InfoHandler::removePub((PublisherBase *)(param_1 + 0xc0),(PublisherInfo *)param_2);
  bVar1 = (((byte)local_70[0] ^ 1) & 1) == 0;
  if (!bVar1) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_18,"removePub failed : {}\n");
    local_8 = &PTR_s_/home/pi/temp/zos_turing/project_002a8b78;
                    /* try { // try from 0021e298 to 0021e29b has its CatchHandler @ 0021e2b8 */
    error<>::error(auStack_20,local_18,uStack_10,auStack_68,&local_8);
  }
  HandleResult::~HandleResult(local_70);
  return bVar1;
}


/* zos::manager::removeSub(zos::SubscriberBase*, zos::SubscriberInfo) */

bool zos::manager::removeSub(long param_1,SubscriberBase *param_2)

{
  bool bVar1;
  char *pcVar2;
  HandleResult local_70 [8];
  undefined1 auStack_68 [32];
  undefined1 auStack_48 [8];
  undefined8 local_40;
  undefined8 uStack_38;
  void *local_30;
  undefined **local_28;
  undefined1 auStack_20 [8];
  undefined8 local_18;
  undefined8 uStack_10;
  undefined **local_8;
  
  pcVar2 = getenv("ZOS_DEBUG_MANAGER");
  if (pcVar2 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_40,"removeSub address : {}\n");
    local_30 = fmt::v9::ptr<>(param_2);
    local_28 = &PTR_s_/home/pi/temp/zos_turing/project_002a8b90;
    status<>::status(auStack_48,local_40,uStack_38,&local_30,&local_28);
  }
  InfoHandler::removeSub((SubscriberBase *)(param_1 + 0xc0),(SubscriberInfo *)param_2);
  bVar1 = (((byte)local_70[0] ^ 1) & 1) == 0;
  if (!bVar1) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_18,"removeSub failed : {}\n");
    local_8 = &PTR_s_/home/pi/temp/zos_turing/project_002a8ba8;
                    /* try { // try from 0021e3cc to 0021e3cf has its CatchHandler @ 0021e3ec */
    error<>::error(auStack_20,local_18,uStack_10,auStack_68,&local_8);
  }
  HandleResult::~HandleResult(local_70);
  return bVar1;
}


/* zos::manager::updatePub(zos::PublisherBase*, zos::PublisherInfo) */

bool __thiscall zos::manager::updatePub(manager *this,PublisherBase *param_1,long param_3)

{
  bool bVar1;
  char *pcVar2;
  HandleResult local_70 [8];
  undefined1 auStack_68 [32];
  undefined1 auStack_48 [8];
  undefined8 local_40;
  undefined8 uStack_38;
  void *local_30;
  undefined **local_28;
  undefined1 auStack_20 [8];
  undefined8 local_18;
  undefined8 uStack_10;
  undefined **local_8;
  
  pcVar2 = getenv("ZOS_DEBUG_MANAGER");
  if (pcVar2 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_40,"updatePub address : {},key-{}\n");
    local_30 = fmt::v9::ptr<>(param_1);
    local_28 = &PTR_s_/home/pi/temp/zos_turing/project_002a8bc0;
    status<>::status(auStack_48,local_40,uStack_38,&local_30,param_3 + 0x40,&local_28);
  }
  InfoHandler::updatePub((PublisherBase *)(this + 0xc0),(PublisherInfo *)param_1);
  bVar1 = (((byte)local_70[0] ^ 1) & 1) == 0;
  if (!bVar1) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_18,"updatePub failed : {}\n");
    local_8 = &PTR_s_/home/pi/temp/zos_turing/project_002a8bd8;
                    /* try { // try from 0021e50c to 0021e50f has its CatchHandler @ 0021e52c */
    error<>::error(auStack_20,local_18,uStack_10,auStack_68,&local_8);
  }
  HandleResult::~HandleResult(local_70);
  return bVar1;
}


/* zos::manager::updateSub(zos::SubscriberBase*, zos::SubscriberInfo) */

bool __thiscall zos::manager::updateSub(manager *this,SubscriberBase *param_1,long param_3)

{
  bool bVar1;
  char *pcVar2;
  HandleResult local_70 [8];
  undefined1 auStack_68 [32];
  undefined1 auStack_48 [8];
  undefined8 local_40;
  undefined8 uStack_38;
  void *local_30;
  undefined **local_28;
  undefined1 auStack_20 [8];
  undefined8 local_18;
  undefined8 uStack_10;
  undefined **local_8;
  
  pcVar2 = getenv("ZOS_DEBUG_MANAGER");
  if (pcVar2 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_40,"updateSub address : {},key-{}\n");
    local_30 = fmt::v9::ptr<>(param_1);
    local_28 = &PTR_s_/home/pi/temp/zos_turing/project_002a8bf0;
    status<>::status(auStack_48,local_40,uStack_38,&local_30,param_3 + 0x48,&local_28);
  }
  InfoHandler::updateSub((SubscriberBase *)(this + 0xc0),(SubscriberInfo *)param_1);
  bVar1 = (((byte)local_70[0] ^ 1) & 1) == 0;
  if (!bVar1) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_18,"updateSub failed : {}\n");
    local_8 = &PTR_s_/home/pi/temp/zos_turing/project_002a8c08;
                    /* try { // try from 0021e64c to 0021e64f has its CatchHandler @ 0021e66c */
    error<>::error(auStack_20,local_18,uStack_10,auStack_68,&local_8);
  }
  HandleResult::~HandleResult(local_70);
  return bVar1;
}


/* zos::manager::uuid[abi:cxx11]() const */

void zos::manager::uuid[abi:cxx11](void)

{
  long in_x0;
  string *in_x8;
  
  std::__cxx11::string::string(in_x8,(string *)(in_x0 + 0x48));
  return;
}


/* zos::manager::pid() const */

undefined4 __thiscall zos::manager::pid(manager *this)

{
  return *(undefined4 *)(this + 0x40);
}


/* zos::manager::node_name[abi:cxx11]() const */

void zos::manager::node_name[abi:cxx11](void)

{
  string *in_x0;
  string *in_x8;
  
  std::__cxx11::string::string(in_x8,in_x0);
  return;
}


/* zos::manager::node_ns[abi:cxx11]() const */

void zos::manager::node_ns[abi:cxx11](void)

{
  long in_x0;
  string *in_x8;
  
  std::__cxx11::string::string(in_x8,(string *)(in_x0 + 0x20));
  return;
}


/* zos::InfoHandler::InfoHandler() */

void __thiscall zos::InfoHandler::InfoHandler(InfoHandler *this)

{
  Info<>::Info((Info<> *)this);
  std::map<>::map((map<> *)(this + 0x60));
  std::map<>::map((map<> *)(this + 0x90));
  std::mutex::mutex((mutex *)(this + 0xc0));
  std::mutex::mutex((mutex *)(this + 0xf0));
  return;
}


/* zos::InfoHandler::~InfoHandler() */

void __thiscall zos::InfoHandler::~InfoHandler(InfoHandler *this)

{
  std::map<>::~map((map<> *)(this + 0x90));
  std::map<>::~map((map<> *)(this + 0x60));
  Info<>::~Info((Info<> *)this);
  return;
}


/* zos::InfoHandler::addPub(zos::PublisherBase*, zos::PublisherInfo const&) */

void zos::InfoHandler::addPub(PublisherBase *param_1,PublisherInfo *param_2)

{
  bool bVar1;
  uint uVar2;
  char *pcVar3;
  long in_x2;
  byte *in_x8;
  PublisherBase *pPVar4;
  undefined8 local_220;
  undefined8 local_218;
  undefined8 local_210;
  undefined8 local_208;
  undefined8 local_200;
  undefined8 local_1f8;
  undefined8 local_1f0;
  undefined8 uStack_1e8;
  undefined8 local_1e0;
  undefined8 uStack_1d8;
  undefined8 local_1d0;
  undefined8 uStack_1c8;
  unique_lock<std::mutex> auStack_1c0 [16];
  undefined1 auStack_1b0 [8];
  undefined8 local_1a8;
  undefined8 uStack_1a0;
  void *local_198;
  undefined **local_190;
  SubscriberInfo aSStack_188 [288];
  SubscriberInfo *local_68;
  type *local_60;
  pair *local_58;
  type *local_50;
  type *local_48;
  pair *local_40;
  map<> *local_38;
  type *local_30;
  type *local_28;
  pair *local_20;
  map<> *local_18;
  map<> *local_10;
  PublisherInfo *local_8;
  
  std::unique_lock<std::mutex>::unique_lock(auStack_1c0,(mutex *)(param_1 + 0xc0));
  pcVar3 = getenv("ZOS_DEBUG_INFO");
  if (pcVar3 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_1a8,"info : addPub-{}-{}\n");
                    /* try { // try from 00222d94 to 00222ddb has its CatchHandler @ 00223160 */
    local_198 = fmt::v9::ptr<>((PublisherBase *)param_2);
    local_190 = &PTR_s_/home/pi/temp/zos_turing/project_002a8c98;
    log<>::log(auStack_1b0,local_1a8,uStack_1a0,in_x2 + 0xa8,&local_198,&local_190);
  }
  Info<>::addPub(param_1,param_2);
  if (((*in_x8 ^ 1) & 1) == 0) {
                    /* try { // try from 00222dfc to 00222eb3 has its CatchHandler @ 0022314c */
    local_8 = (PublisherInfo *)Info<>::getPub((Info<> *)param_1,(PublisherBase *)param_2);
    local_10 = (map<> *)(param_1 + 0x30);
    local_1f8 = std::map<>::begin(local_10);
    local_200 = std::map<>::end(local_10);
    while (uVar2 = std::operator==((_Rb_tree_iterator *)&local_1f8,(_Rb_tree_iterator *)&local_2 00),
          ((uVar2 & 0xff ^ 1) & 1) != 0) {
      local_58 = (pair *)std::_Rb_tree_iterator<>::operator*((_Rb_tree_iterator<> *)&local_1f8);
      local_60 = std::get<>(local_58);
      local_68 = (SubscriberInfo *)std::get<>(local_58);
      bVar1 = std::operator==((string *)(in_x2 + 0xa8),(string *)(local_68 + 0xb0));
      if (bVar1) {
        PublisherInfo::addSubs(local_8,local_68);
        std::unique_lock<std::mutex>::unlock(auStack_1c0);
        PublisherBase::link(*(PublisherBase **)(in_x2 + 0x108),*(SubscriberBase **)local_60);
        std::unique_lock<std::mutex>::lock(auStack_1c0);
      }
      std::_Rb_tree_iterator<>::operator++((_Rb_tree_iterator<> *)&local_1f8);
    }
    local_1f0 = 0;
    uStack_1e8 = 0;
    local_1e0 = 0;
    uStack_1d8 = 0;
    local_1d0 = 0;
    uStack_1c8 = 0;
    std::set<>::set((set<> *)&local_1f0);
    local_18 = (map<> *)(param_1 + 0x60);
    local_208 = std::map<>::begin(local_18);
    local_210 = std::map<>::end(local_18);
    while (uVar2 = std::operator==((_Rb_tree_iterator *)&local_208,(_Rb_tree_iterator *)&local_2 10),
          ((uVar2 & 0xff ^ 1) & 1) != 0) {
      local_20 = (pair *)std::_Rb_tree_iterator<>::operator*((_Rb_tree_iterator<> *)&local_208);
      local_28 = std::get<>(local_20);
      local_30 = std::get<>(local_20);
      local_38 = (map<> *)(local_30 + 0x38);
      local_218 = std::map<>::begin(local_38);
      local_220 = std::map<>::end(local_38);
      while (uVar2 = std::operator==((_Rb_tree_iterator *)&local_218,(_Rb_tree_iterator *)&local _220
                                    ), ((uVar2 & 0xff ^ 1) & 1) != 0) {
        local_40 = (pair *)std::_Rb_tree_iterator<>::operator*((_Rb_tree_iterator<> *)&local_218 );
        local_48 = std::get<>(local_40);
        local_50 = std::get<>(local_40);
        bVar1 = std::operator==((string *)(in_x2 + 0xa8),(string *)(local_50 + 0xb0));
                    /* try { // try from 00222fec to 00223087 has its CatchHandler @ 0022313c */
        if (((bVar1) &&
            (bVar1 = std::operator==((string *)(in_x2 + 0xe8),(string *)(local_50 + 0xf0)), bVar1 ))
           && (uVar2 = PublisherInfo::hasSub(local_8,(SubscriberInfo *)local_50),
              ((uVar2 & 0xff ^ 1) & 1) != 0)) {
          bVar1 = true;
        }
        else {
          bVar1 = false;
        }
        if (bVar1) {
          uVar2 = std::__cxx11::string::empty((string *)local_50);
          if ((((uVar2 & 0xff ^ 1) & 1) == 0) || (*(int *)(local_50 + 0x20) == 0)) {
            bVar1 = false;
          }
          else {
            bVar1 = true;
          }
          if (bVar1) {
            PublisherInfo::addSubs(local_8,(SubscriberInfo *)local_50);
            pPVar4 = *(PublisherBase **)(in_x2 + 0x108);
            SubscriberInfo::SubscriberInfo(aSStack_188,(SubscriberInfo *)local_50);
                    /* try { // try from 00223098 to 0022309b has its CatchHandler @ 0022312c */
            PublisherBase::link_u(pPVar4,0,aSStack_188);
            SubscriberInfo::~SubscriberInfo(aSStack_188);
                    /* try { // try from 002230b0 to 002230b3 has its CatchHandler @ 0022313c */
            std::set<>::insert((set<> *)&local_1f0,(void **)(in_x2 + 0x108));
          }
        }
        std::_Rb_tree_iterator<>::operator++((_Rb_tree_iterator<> *)&local_218);
      }
      std::_Rb_tree_iterator<>::operator++((_Rb_tree_iterator<> *)&local_208);
    }
    std::set<>::~set((set<> *)&local_1f0);
  }
  std::unique_lock<std::mutex>::~unique_lock(auStack_1c0);
  return;
}


/* zos::InfoHandler::addSub(zos::SubscriberBase*, zos::SubscriberInfo const&) */

void zos::InfoHandler::addSub(SubscriberBase *param_1,SubscriberInfo *param_2)

{
  bool bVar1;
  uint uVar2;
  char *pcVar3;
  SubscriberInfo *in_x2;
  byte *in_x8;
  undefined8 local_98;
  undefined8 local_90;
  undefined8 local_88;
  undefined8 uStack_80;
  undefined8 local_78;
  undefined8 uStack_70;
  undefined8 local_68;
  undefined8 uStack_60;
  unique_lock<std::mutex> auStack_58 [16];
  undefined1 auStack_48 [8];
  undefined8 local_40;
  undefined8 uStack_38;
  void *local_30;
  undefined **local_28;
  PublisherInfo *local_20;
  type *local_18;
  pair *local_10;
  map<> *local_8;
  
  std::unique_lock<std::mutex>::unique_lock(auStack_58,(mutex *)(param_1 + 0xc0));
  pcVar3 = getenv("ZOS_DEBUG_INFO");
  if (pcVar3 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_40,"info : addSub-{}-{}\n");
                    /* try { // try from 002231f8 to 0022323f has its CatchHandler @ 00223384 */
    local_30 = fmt::v9::ptr<>((SubscriberBase *)param_2);
    local_28 = &PTR_s_/home/pi/temp/zos_turing/project_002a8cb0;
    log<>::log(auStack_48,local_40,uStack_38,in_x2 + 0xb0,&local_30,&local_28);
  }
  Info<>::addSub(param_1,param_2);
  if (((*in_x8 ^ 1) & 1) == 0) {
    local_88 = 0;
    uStack_80 = 0;
    local_78 = 0;
    uStack_70 = 0;
    local_68 = 0;
    uStack_60 = 0;
    std::set<>::set((set<> *)&local_88);
    local_8 = (map<> *)param_1;
    local_90 = std::map<>::begin((map<> *)param_1);
    local_98 = std::map<>::end(local_8);
    while (uVar2 = std::operator==((_Rb_tree_iterator *)&local_90,(_Rb_tree_iterator *)&local_98 ),
          ((uVar2 & 0xff ^ 1) & 1) != 0) {
      local_10 = (pair *)std::_Rb_tree_iterator<>::operator*((_Rb_tree_iterator<> *)&local_90);
      local_18 = std::get<>(local_10);
      local_20 = (PublisherInfo *)std::get<>(local_10);
      bVar1 = std::operator==((string *)(local_20 + 0xa8),(string *)(in_x2 + 0xb0));
      if (bVar1) {
                    /* try { // try from 002232e8 to 0022331b has its CatchHandler @ 00223368 */
        PublisherInfo::addSubs(local_20,in_x2);
        PublisherBase::link(*(PublisherBase **)(local_20 + 0x108),false,
                            *(SubscriberBase **)(in_x2 + 0x110));
        std::set<>::insert((set<> *)&local_88,(void **)(local_20 + 0x108));
      }
      std::_Rb_tree_iterator<>::operator++((_Rb_tree_iterator<> *)&local_90);
    }
    std::set<>::~set((set<> *)&local_88);
  }
  std::unique_lock<std::mutex>::~unique_lock(auStack_58);
  return;
}


/* zos::InfoHandler::addOuter(zos::ProcessInfo const&) */

void zos::InfoHandler::addOuter(ProcessInfo *param_1)

{
  uint uVar1;
  pair<> *this;
  ProcessInfo *this_00;
  string *in_x1;
  HandleResult *in_x8;
  undefined8 local_170;
  string asStack_168 [32];
  unique_lock<std::mutex> auStack_148 [16];
  undefined8 local_138;
  string asStack_130 [32];
  allocator aaStack_110 [8];
  pair apStack_108 [104];
  time_t local_a0;
  undefined8 local_98;
  undefined8 uStack_90;
  undefined8 uStack_88;
  undefined8 uStack_80;
  undefined8 local_78;
  undefined8 uStack_70;
  undefined8 uStack_68;
  undefined8 uStack_60;
  undefined8 local_58;
  undefined8 uStack_50;
  undefined8 uStack_48;
  undefined8 uStack_40;
  string asStack_38 [32];
  allocator aaStack_18 [8];
  allocator *local_10;
  allocator *local_8;
  
  std::unique_lock<std::mutex>::unique_lock(auStack_148,(mutex *)(param_1 + 0xc0));
                    /* try { // try from 00223adc to 00223adf has its CatchHandler @ 00223cc4 */
  std::__cxx11::string::string(asStack_168,in_x1);
                    /* try { // try from 00223aec to 00223aef has its CatchHandler @ 00223cb4 */
  local_170 = std::map<>::find((map<> *)(param_1 + 0x60),asStack_168);
  local_138 = std::map<>::end((map<> *)(param_1 + 0x60));
  uVar1 = std::operator==((_Rb_tree_iterator *)&local_170,(_Rb_tree_iterator *)&local_138);
  if (((uVar1 & 0xff ^ 1) & 1) == 0) {
    local_a0 = time((time_t *)0x0);
    uStack_90 = 0;
    local_98 = 0;
    uStack_80 = 0;
    uStack_88 = 0;
    uStack_70 = 0;
    local_78 = 0;
    uStack_60 = 0;
    uStack_68 = 0;
    uStack_50 = 0;
    local_58 = 0;
    uStack_40 = 0;
    uStack_48 = 0;
    Info<>::Info((Info<> *)&local_98);
    std::make_pair<>(&local_a0,(Info *)&local_98);
                    /* try { // try from 00223bbc to 00223bbf has its CatchHandler @ 00223c7c */
    this = (pair<> *)std::map<>::operator[]((map<> *)(param_1 + 0x60),asStack_168);
    std::pair<>::operator=(this,apStack_108);
    std::pair<>::~pair((pair<> *)apStack_108);
    Info<>::~Info((Info<> *)&local_98);
                    /* try { // try from 00223bf0 to 00223bfb has its CatchHandler @ 00223cb4 */
    this_00 = (ProcessInfo *)std::map<>::operator[]((map<> *)(param_1 + 0x90),asStack_168);
    ProcessInfo::operator=(this_00,(ProcessInfo *)in_x1);
    local_10 = aaStack_18;
                    /* try { // try from 00223c24 to 00223c27 has its CatchHandler @ 00223ca8 */
    std::__cxx11::string::string<>(asStack_38,"success",aaStack_18);
                    /* try { // try from 00223c38 to 00223c3b has its CatchHandler @ 00223c94 */
    HandleResult::HandleResult(in_x8,true,asStack_38);
    std::__cxx11::string::~string(asStack_38);
  }
  else {
    local_8 = aaStack_110;
                    /* try { // try from 00223b50 to 00223b53 has its CatchHandler @ 00223c70 */
    std::__cxx11::string::string<>(asStack_130,"uuid already exist",aaStack_110);
                    /* try { // try from 00223b64 to 00223b67 has its CatchHandler @ 00223c5c */
    HandleResult::HandleResult(in_x8,false,asStack_130);
    std::__cxx11::string::~string(asStack_130);
  }
  std::__cxx11::string::~string(asStack_168);
  std::unique_lock<std::mutex>::~unique_lock(auStack_148);
  return;
}


/* zos::InfoHandler::updatePub(zos::PublisherBase*, zos::PublisherInfo const&) */

void zos::InfoHandler::updatePub(PublisherBase *param_1,PublisherInfo *param_2)

{
  char *pcVar1;
  PublisherInfo *in_x2;
  unique_lock<std::mutex> auStack_180 [16];
  undefined1 auStack_170 [8];
  undefined8 local_168;
  undefined8 uStack_160;
  void *local_158;
  undefined **local_150;
  PublisherInfo aPStack_148 [328];
  
  std::unique_lock<std::mutex>::unique_lock(auStack_180,(mutex *)(param_1 + 0xc0));
  pcVar1 = getenv("ZOS_DEBUG_INFO");
  if (pcVar1 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_168,"info : updatePub-{}-{}\n");
                    /* try { // try from 002238ec to 0022392f has its CatchHandler @ 00223970 */
    local_158 = fmt::v9::ptr<>((PublisherBase *)param_2);
    local_150 = &PTR_s_/home/pi/temp/zos_turing/project_002a8cf8;
    log<>::log(auStack_170,local_168,uStack_160,in_x2 + 0xa8,&local_158,&local_150);
  }
  PublisherInfo::PublisherInfo(aPStack_148,in_x2);
                    /* try { // try from 00223944 to 00223947 has its CatchHandler @ 00223960 */
  Info<>::updatePub(param_1,param_2,aPStack_148);
  PublisherInfo::~PublisherInfo(aPStack_148);
  std::unique_lock<std::mutex>::~unique_lock(auStack_180);
  return;
}


/* zos::InfoHandler::updateSub(zos::SubscriberBase*, zos::SubscriberInfo const&) */

void zos::InfoHandler::updateSub(SubscriberBase *param_1,SubscriberInfo *param_2)

{
  char *pcVar1;
  SubscriberInfo *in_x2;
  unique_lock<std::mutex> auStack_158 [16];
  undefined1 auStack_148 [8];
  undefined8 local_140;
  undefined8 uStack_138;
  void *local_130;
  undefined **local_128;
  SubscriberInfo aSStack_120 [288];
  
  std::unique_lock<std::mutex>::unique_lock(auStack_158,(mutex *)(param_1 + 0xc0));
  pcVar1 = getenv("ZOS_DEBUG_INFO");
  if (pcVar1 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_140,"info : updateSub-{}-{}\n");
                    /* try { // try from 00223a04 to 00223a47 has its CatchHandler @ 00223a88 */
    local_130 = fmt::v9::ptr<>((SubscriberBase *)param_2);
    local_128 = &PTR_s_/home/pi/temp/zos_turing/project_002a8d10;
    log<>::log(auStack_148,local_140,uStack_138,in_x2 + 0xb0,&local_130,&local_128);
  }
  SubscriberInfo::SubscriberInfo(aSStack_120,in_x2);
                    /* try { // try from 00223a5c to 00223a5f has its CatchHandler @ 00223a78 */
  Info<>::updateSub(param_1,param_2,aSStack_120);
  SubscriberInfo::~SubscriberInfo(aSStack_120);
  std::unique_lock<std::mutex>::~unique_lock(auStack_158);
  return;
}


/* zos::InfoHandler::removePub(zos::PublisherBase*, zos::PublisherInfo const&) */

void zos::InfoHandler::removePub(PublisherBase *param_1,PublisherInfo *param_2)

{
  uint uVar1;
  char *pcVar2;
  PublisherInfo *in_x2;
  HandleResult *in_x8;
  undefined8 local_a0;
  undefined8 local_98;
  unique_lock<std::mutex> auStack_90 [16];
  undefined1 auStack_80 [8];
  undefined8 local_78;
  undefined8 uStack_70;
  void *local_68;
  undefined **local_60;
  string asStack_58 [32];
  allocator aaStack_38 [8];
  allocator *local_30;
  SubscriberInfo *local_28;
  type *local_20;
  pair *local_18;
  map<> *local_10;
  PublisherInfo *local_8;
  
  std::unique_lock<std::mutex>::unique_lock(auStack_90,(mutex *)(param_1 + 0xc0));
  pcVar2 = getenv("ZOS_DEBUG_INFO");
  if (pcVar2 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_78,"info : removePub-{}-{}\n");
                    /* try { // try from 00223418 to 00223457 has its CatchHandler @ 002235cc */
    local_68 = fmt::v9::ptr<>((PublisherBase *)param_2);
    local_60 = &PTR_s_/home/pi/temp/zos_turing/project_002a8cc8;
    log<>::log(auStack_80,local_78,uStack_70,in_x2 + 0xa8,&local_68,&local_60);
  }
  uVar1 = Info<>::checkPub((Info<> *)param_1,(PublisherBase *)param_2);
  if (((uVar1 & 0xff ^ 1) & 1) == 0) {
                    /* try { // try from 002234c4 to 0022359f has its CatchHandler @ 002235cc */
    local_8 = (PublisherInfo *)Info<>::getPub((Info<> *)param_1,(PublisherBase *)param_2);
    local_10 = (map<> *)(param_1 + 0x30);
    local_98 = std::map<>::begin(local_10);
    local_a0 = std::map<>::end(local_10);
    while (uVar1 = std::operator==((_Rb_tree_iterator *)&local_98,(_Rb_tree_iterator *)&local_a0 ),
          ((uVar1 & 0xff ^ 1) & 1) != 0) {
      local_18 = (pair *)std::_Rb_tree_iterator<>::operator*((_Rb_tree_iterator<> *)&local_98);
      local_20 = std::get<>(local_18);
      local_28 = (SubscriberInfo *)std::get<>(local_18);
      uVar1 = PublisherInfo::hasSub(local_8,local_28);
      if ((uVar1 & 1) != 0) {
        PublisherInfo::removeSubs(local_8,local_28);
        PublisherBase::unlink
                  (*(PublisherBase **)(in_x2 + 0x108),false,*(SubscriberBase **)(local_28 + 0x110 ));
      }
      std::_Rb_tree_iterator<>::operator++((_Rb_tree_iterator<> *)&local_98);
    }
    Info<>::removePub((Info<> *)param_1,(PublisherBase *)param_2,in_x2);
  }
  else {
    local_30 = aaStack_38;
                    /* try { // try from 00223498 to 0022349b has its CatchHandler @ 002235c0 */
    std::__cxx11::string::string<>(asStack_58,"publisher not found",aaStack_38);
                    /* try { // try from 002234ac to 002234af has its CatchHandler @ 002235ac */
    HandleResult::HandleResult(in_x8,false,asStack_58);
    std::__cxx11::string::~string(asStack_58);
  }
  std::unique_lock<std::mutex>::~unique_lock(auStack_90);
  return;
}


/* zos::InfoHandler::removeSub(zos::SubscriberBase*, zos::SubscriberInfo const&) */

void zos::InfoHandler::removeSub(SubscriberBase *param_1,SubscriberInfo *param_2)

{
  uint uVar1;
  char *pcVar2;
  SubscriberInfo *in_x2;
  HandleResult *in_x8;
  undefined8 local_f0;
  undefined8 local_e8;
  HandleResult aHStack_e0 [40];
  undefined8 local_b8;
  undefined8 uStack_b0;
  undefined8 local_a8;
  undefined8 uStack_a0;
  undefined8 local_98;
  undefined8 uStack_90;
  unique_lock<std::mutex> auStack_88 [16];
  undefined1 auStack_78 [8];
  undefined8 local_70;
  undefined8 uStack_68;
  void *local_60;
  undefined **local_58;
  string asStack_50 [32];
  allocator aaStack_30 [8];
  allocator *local_28;
  PublisherInfo *local_20;
  type *local_18;
  pair *local_10;
  map<> *local_8;
  
  std::unique_lock<std::mutex>::unique_lock(auStack_88,(mutex *)(param_1 + 0xc0));
  pcVar2 = getenv("ZOS_DEBUG_INFO");
  if (pcVar2 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_70,"info : removeSub-{}-{}\n");
                    /* try { // try from 00223660 to 0022369f has its CatchHandler @ 00223858 */
    local_60 = fmt::v9::ptr<>((SubscriberBase *)param_2);
    local_58 = &PTR_s_/home/pi/temp/zos_turing/project_002a8ce0;
    log<>::log(auStack_78,local_70,uStack_68,in_x2 + 0xb0,&local_60,&local_58);
  }
  uVar1 = Info<>::checkSub((Info<> *)param_1,(SubscriberBase *)param_2);
  if (((uVar1 & 0xff ^ 1) & 1) == 0) {
    local_b8 = 0;
    uStack_b0 = 0;
    local_a8 = 0;
    uStack_a0 = 0;
    local_98 = 0;
    uStack_90 = 0;
    std::set<>::set((set<> *)&local_b8);
    local_8 = (map<> *)param_1;
    local_e8 = std::map<>::begin((map<> *)param_1);
    local_f0 = std::map<>::end(local_8);
    while (uVar1 = std::operator==((_Rb_tree_iterator *)&local_e8,(_Rb_tree_iterator *)&local_f0 ),
          ((uVar1 & 0xff ^ 1) & 1) != 0) {
      local_10 = (pair *)std::_Rb_tree_iterator<>::operator*((_Rb_tree_iterator<> *)&local_e8);
      local_18 = std::get<>(local_10);
      local_20 = (PublisherInfo *)std::get<>(local_10);
                    /* try { // try from 00223768 to 002237fb has its CatchHandler @ 00223848 */
      uVar1 = PublisherInfo::hasSub(local_20,in_x2);
      if ((uVar1 & 1) != 0) {
        PublisherInfo::removeSubs(local_20,in_x2);
        PublisherBase::unlink
                  (*(PublisherBase **)(local_20 + 0x108),false,*(SubscriberBase **)(in_x2 + 0x110 ));
        std::set<>::insert((set<> *)&local_b8,(void **)(local_20 + 0x108));
      }
      std::_Rb_tree_iterator<>::operator++((_Rb_tree_iterator<> *)&local_e8);
    }
    Info<>::removeSub((Info<> *)param_1,(SubscriberBase *)param_2,in_x2);
    HandleResult::HandleResult(in_x8,aHStack_e0);
    HandleResult::~HandleResult(aHStack_e0);
    std::set<>::~set((set<> *)&local_b8);
  }
  else {
    local_28 = aaStack_30;
                    /* try { // try from 002236e0 to 002236e3 has its CatchHandler @ 0022383c */
    std::__cxx11::string::string<>(asStack_50,"subscriber not found",aaStack_30);
                    /* try { // try from 002236f4 to 002236f7 has its CatchHandler @ 00223828 */
    HandleResult::HandleResult(in_x8,false,asStack_50);
    std::__cxx11::string::~string(asStack_50);
  }
  std::unique_lock<std::mutex>::~unique_lock(auStack_88);
  return;
}


/* zos::InfoHandler::updateOuterPub(zos::PublisherInfo const&) */

void zos::InfoHandler::updateOuterPub(PublisherInfo *param_1)

{
  uint uVar1;
  long lVar2;
  time_t tVar3;
  PublisherInfo *in_x1;
  HandleResult *in_x8;
  HandleResult local_1e0 [40];
  undefined8 local_1b8;
  unique_lock<std::mutex> auStack_1b0 [16];
  undefined8 local_1a0;
  string asStack_198 [32];
  allocator aaStack_178 [8];
  string asStack_170 [32];
  PublisherInfo aPStack_150 [328];
  allocator *local_8;
  
  std::unique_lock<std::mutex>::unique_lock(auStack_1b0,(mutex *)(param_1 + 0xc0));
                    /* try { // try from 00224554 to 00224557 has its CatchHandler @ 002246b8 */
  local_1b8 = std::map<>::find((map<> *)(param_1 + 0x60),(string *)(in_x1 + 0x20));
  local_1a0 = std::map<>::end((map<> *)(param_1 + 0x60));
  uVar1 = std::operator==((_Rb_tree_iterator *)&local_1b8,(_Rb_tree_iterator *)&local_1a0);
  if ((uVar1 & 1) == 0) {
    lVar2 = std::_Rb_tree_iterator<>::operator->((_Rb_tree_iterator<> *)&local_1b8);
                    /* try { // try from 002245ec to 002245ef has its CatchHandler @ 002246b8 */
    std::__cxx11::string::string(asStack_170,(string *)(in_x1 + 0x88));
                    /* try { // try from 002245f8 to 002245fb has its CatchHandler @ 002246a8 */
    PublisherInfo::PublisherInfo(aPStack_150,in_x1);
                    /* try { // try from 00224618 to 0022461b has its CatchHandler @ 00224698 */
    Info<>::updatePub(local_1e0,lVar2 + 0x28,asStack_170,aPStack_150);
    PublisherInfo::~PublisherInfo(aPStack_150);
    std::__cxx11::string::~string(asStack_170);
    if (((byte)local_1e0[0] & 1) != 0) {
      tVar3 = time((time_t *)0x0);
      lVar2 = std::_Rb_tree_iterator<>::operator->((_Rb_tree_iterator<> *)&local_1b8);
      *(time_t *)(lVar2 + 0x20) = tVar3;
    }
    HandleResult::HandleResult(in_x8,local_1e0);
    HandleResult::~HandleResult(local_1e0);
  }
  else {
    local_8 = aaStack_178;
                    /* try { // try from 002245b0 to 002245b3 has its CatchHandler @ 0022468c */
    std::__cxx11::string::string<>(asStack_198,"uuid not exist",aaStack_178);
                    /* try { // try from 002245c4 to 002245c7 has its CatchHandler @ 00224678 */
    HandleResult::HandleResult(in_x8,false,asStack_198);
    std::__cxx11::string::~string(asStack_198);
  }
  std::unique_lock<std::mutex>::~unique_lock(auStack_1b0);
  return;
}


/* zos::InfoHandler::removeOuterPub(zos::PublisherInfo const&) */

void zos::InfoHandler::removeOuterPub(PublisherInfo *param_1)

{
  uint uVar1;
  long lVar2;
  time_t tVar3;
  long in_x1;
  HandleResult *in_x8;
  Info<> local_98 [40];
  undefined8 local_70;
  unique_lock<std::mutex> auStack_68 [16];
  undefined8 local_58;
  string asStack_50 [32];
  allocator aaStack_30 [8];
  string asStack_28 [32];
  allocator *local_8;
  
  std::unique_lock<std::mutex>::unique_lock(auStack_68,(mutex *)(param_1 + 0xc0));
                    /* try { // try from 00224088 to 0022408b has its CatchHandler @ 002241c4 */
  local_70 = std::map<>::find((map<> *)(param_1 + 0x60),(string *)(in_x1 + 0x20));
  local_58 = std::map<>::end((map<> *)(param_1 + 0x60));
  uVar1 = std::operator==((_Rb_tree_iterator *)&local_70,(_Rb_tree_iterator *)&local_58);
  if ((uVar1 & 1) == 0) {
    lVar2 = std::_Rb_tree_iterator<>::operator->((_Rb_tree_iterator<> *)&local_70);
                    /* try { // try from 00224120 to 00224123 has its CatchHandler @ 002241c4 */
    std::__cxx11::string::string(asStack_28,(string *)(in_x1 + 0x88));
                    /* try { // try from 0022413c to 0022413f has its CatchHandler @ 002241b4 */
    Info<>::removePub(local_98,lVar2 + 0x28,asStack_28,in_x1);
    std::__cxx11::string::~string(asStack_28);
    if (((byte)local_98[0] & 1) != 0) {
      tVar3 = time((time_t *)0x0);
      lVar2 = std::_Rb_tree_iterator<>::operator->((_Rb_tree_iterator<> *)&local_70);
      *(time_t *)(lVar2 + 0x20) = tVar3;
    }
    HandleResult::HandleResult(in_x8,(HandleResult *)local_98);
    HandleResult::~HandleResult((HandleResult *)local_98);
  }
  else {
    local_8 = aaStack_30;
                    /* try { // try from 002240e4 to 002240e7 has its CatchHandler @ 002241a8 */
    std::__cxx11::string::string<>(asStack_50,"uuid not exist",aaStack_30);
                    /* try { // try from 002240f8 to 002240fb has its CatchHandler @ 00224194 */
    HandleResult::HandleResult(in_x8,true,asStack_50);
    std::__cxx11::string::~string(asStack_50);
  }
  std::unique_lock<std::mutex>::~unique_lock(auStack_68);
  return;
}


/* zos::InfoHandler::removeOuter(zos::ProcessInfo const&) */

void zos::InfoHandler::removeOuter(ProcessInfo *param_1)

{
  bool bVar1;
  uint uVar2;
  long lVar3;
  string *in_x1;
  HandleResult *in_x8;
  PublisherBase *pPVar4;
  undefined8 local_250;
  undefined8 local_248;
  undefined8 local_240;
  undefined8 local_238;
  undefined8 local_230;
  undefined8 uStack_228;
  undefined8 local_220;
  undefined8 uStack_218;
  undefined8 local_210;
  undefined8 uStack_208;
  undefined8 local_200;
  string asStack_1f8 [32];
  unique_lock<std::mutex> auStack_1d8 [16];
  undefined8 local_1c8;
  string asStack_1c0 [32];
  allocator aaStack_1a0 [8];
  SubscriberInfo aSStack_198 [288];
  string asStack_78 [32];
  allocator aaStack_58 [8];
  allocator *local_50;
  allocator *local_48;
  PublisherInfo *local_40;
  type *local_38;
  pair *local_30;
  map<> *local_28;
  type *local_20;
  type *local_18;
  pair *local_10;
  map<> *local_8;
  
  std::unique_lock<std::mutex>::unique_lock(auStack_1d8,(mutex *)(param_1 + 0xc0));
                    /* try { // try from 00223d1c to 00223d1f has its CatchHandler @ 00224020 */
  std::__cxx11::string::string(asStack_1f8,in_x1);
                    /* try { // try from 00223d2c to 00223d2f has its CatchHandler @ 00224010 */
  local_200 = std::map<>::find((map<> *)(param_1 + 0x60),asStack_1f8);
  local_1c8 = std::map<>::end((map<> *)(param_1 + 0x60));
  uVar2 = std::operator==((_Rb_tree_iterator *)&local_200,(_Rb_tree_iterator *)&local_1c8);
  if ((uVar2 & 1) == 0) {
    local_230 = 0;
    uStack_228 = 0;
    local_220 = 0;
    uStack_218 = 0;
    local_210 = 0;
    uStack_208 = 0;
    std::set<>::set((set<> *)&local_230);
    lVar3 = std::_Rb_tree_iterator<>::operator->((_Rb_tree_iterator<> *)&local_200);
    local_8 = (map<> *)(lVar3 + 0x58);
    local_238 = std::map<>::begin(local_8);
    local_240 = std::map<>::end(local_8);
    while (uVar2 = std::operator==((_Rb_tree_iterator *)&local_238,(_Rb_tree_iterator *)&local_2 40),
          ((uVar2 & 0xff ^ 1) & 1) != 0) {
      local_10 = (pair *)std::_Rb_tree_iterator<>::operator*((_Rb_tree_iterator<> *)&local_238);
      local_18 = std::get<>(local_10);
      local_20 = std::get<>(local_10);
      local_28 = (map<> *)param_1;
      local_248 = std::map<>::begin((map<> *)param_1);
      local_250 = std::map<>::end(local_28);
      while (uVar2 = std::operator==((_Rb_tree_iterator *)&local_248,(_Rb_tree_iterator *)&local _250
                                    ), ((uVar2 & 0xff ^ 1) & 1) != 0) {
        local_30 = (pair *)std::_Rb_tree_iterator<>::operator*((_Rb_tree_iterator<> *)&local_248 );
        local_38 = std::get<>(local_30);
        local_40 = (PublisherInfo *)std::get<>(local_30);
        bVar1 = std::operator==((string *)(local_40 + 0xa8),(string *)(local_20 + 0xb0));
        if (bVar1) {
                    /* try { // try from 00223e8c to 00223ea3 has its CatchHandler @ 00224000 */
          PublisherInfo::removeSubs(local_40,(SubscriberInfo *)local_20);
          pPVar4 = *(PublisherBase **)(local_40 + 0x108);
          SubscriberInfo::SubscriberInfo(aSStack_198,(SubscriberInfo *)local_20);
                    /* try { // try from 00223eb4 to 00223eb7 has its CatchHandler @ 00223fd0 */
          PublisherBase::unlink_u(pPVar4,0,aSStack_198);
          SubscriberInfo::~SubscriberInfo(aSStack_198);
                    /* try { // try from 00223ecc to 00223f47 has its CatchHandler @ 00224000 */
          std::set<>::insert((set<> *)&local_230,(void **)(local_40 + 0x108));
        }
        std::_Rb_tree_iterator<>::operator++((_Rb_tree_iterator<> *)&local_248);
      }
      std::_Rb_tree_iterator<>::operator++((_Rb_tree_iterator<> *)&local_238);
    }
    std::map<>::erase[abi:cxx11]((map<> *)(param_1 + 0x60),local_200);
    std::map<>::erase((map<> *)(param_1 + 0x90),asStack_1f8);
    local_50 = aaStack_58;
                    /* try { // try from 00223f70 to 00223f73 has its CatchHandler @ 00223ff4 */
    std::__cxx11::string::string<>(asStack_78,"success",aaStack_58);
                    /* try { // try from 00223f84 to 00223f87 has its CatchHandler @ 00223fe0 */
    HandleResult::HandleResult(in_x8,true,asStack_78);
    std::__cxx11::string::~string(asStack_78);
    std::set<>::~set((set<> *)&local_230);
  }
  else {
    local_48 = aaStack_1a0;
                    /* try { // try from 00223d88 to 00223d8b has its CatchHandler @ 00223fc4 */
    std::__cxx11::string::string<>(asStack_1c0,"uuid not exist",aaStack_1a0);
                    /* try { // try from 00223d9c to 00223d9f has its CatchHandler @ 00223fb0 */
    HandleResult::HandleResult(in_x8,true,asStack_1c0);
    std::__cxx11::string::~string(asStack_1c0);
  }
  std::__cxx11::string::~string(asStack_1f8);
  std::unique_lock<std::mutex>::~unique_lock(auStack_1d8);
  return;
}




