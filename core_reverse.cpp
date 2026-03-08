
/* zos::SubscriberBase::SubscriberBase(unsigned int, std::__cxx11::string const&,
   std::__cxx11::string const&, std::function<void (zos::Data const&)> const&) */

void __thiscall
zos::SubscriberBase::SubscriberBase
          (SubscriberBase *this,uint param_1,string *param_2,string *param_3,function *param_4)

{
  string asStack_30 [32];
  allocator aaStack_10 [8];
  allocator *local_8;
  
  local_8 = aaStack_10;
                    /* try { // try from 001dcdb8 to 001dcdbb has its CatchHandler @ 001dce00 */
  std::__cxx11::string::string<>(asStack_30,"",aaStack_10);
                    /* try { // try from 001dcdd8 to 001dcddb has its CatchHandler @ 001dcdec */
  SubscriberBase(this,param_1,asStack_30,param_2,param_3,param_4);
  std::__cxx11::string::~string(asStack_30);
  return;
}


/* zos::SubscriberBase::SubscriberBase(unsigned int, std::__cxx11::string const&,
   std::__cxx11::string const&, std::__cxx11::string const&, std::function<void (zos::Data const&)>
   const&) */

void __thiscall
zos::SubscriberBase::SubscriberBase
          (SubscriberBase *this,uint param_1,string *param_2,string *param_3,string *param_4,
          function *param_5)

{
  SubscriberInfo *pSVar1;
  bool bVar2;
  undefined4 uVar3;
  manager *pmVar4;
  char *pcVar5;
  uint local_26c;
  SubscriberBase *local_268;
  string asStack_258 [32];
  allocator aaStack_238 [8];
  string asStack_230 [32];
  string asStack_210 [32];
  string asStack_1f0 [32];
  string asStack_1d0 [32];
  undefined1 auStack_1b0 [8];
  undefined8 local_1a8;
  undefined8 uStack_1a0;
  void *local_198;
  undefined **local_190;
  _Bind a_Stack_188 [24];
  code *local_170;
  undefined8 local_168;
  SubscriberBase *local_160;
  undefined4 local_154;
  _Bind a_Stack_150 [40];
  SubscriberInfo aSStack_128 [288];
  allocator *local_8;
  
  local_26c = param_1;
  local_268 = this;
  std::atomic<bool>::atomic((atomic<bool> *)this,false);
  std::mutex::mutex((mutex *)(local_268 + 8));
  std::make_unique<>(&local_26c);
  std::function<>::function((function<> *)(local_268 + 0x40));
  std::function<>::function((function<> *)(local_268 + 0x60));
  std::atomic<>::atomic((atomic<> *)(local_268 + 0x80),100);
                    /* try { // try from 001dce98 to 001dce9b has its CatchHandler @ 001dd1dc */
  udp::socket::socket((socket *)(local_268 + 0x88));
  std::mutex::mutex((mutex *)(local_268 + 0xfef0));
  std::atomic<bool>::atomic((atomic<bool> *)(local_268 + 0xff20),false);
  pSVar1 = (SubscriberInfo *)(local_268 + 0xff28);
  local_8 = aaStack_238;
                    /* try { // try from 001dcef4 to 001dcef7 has its CatchHandler @ 001dd18c */
  std::__cxx11::string::string<>(asStack_258,"",aaStack_238);
                    /* try { // try from 001dcef8 to 001dcf0f has its CatchHandler @ 001dd178 */
  Singleton<>::_<>();
  manager::uuid[abi:cxx11]();
                    /* try { // try from 001dcf10 to 001dcf33 has its CatchHandler @ 001dd168 */
  pmVar4 = Singleton<>::_<>();
  uVar3 = manager::pid(pmVar4);
  Singleton<>::_<>();
  manager::node_name[abi:cxx11]();
                    /* try { // try from 001dcf34 to 001dcf4b has its CatchHandler @ 001dd158 */
  Singleton<>::_<>();
  manager::node_ns[abi:cxx11]();
                    /* try { // try from 001dcf54 to 001dcf57 has its CatchHandler @ 001dd148 */
  generateUUID[abi:cxx11]();
                    /* try { // try from 001dcfac to 001dcfaf has its CatchHandler @ 001dd138 */
  SubscriberInfo::SubscriberInfo
            (pSVar1,asStack_258,0,asStack_230,uVar3,asStack_210,asStack_1f0,param_2,asStack_1d0,
             param_3,param_4,local_268,1);
  std::__cxx11::string::~string(asStack_1d0);
  std::__cxx11::string::~string(asStack_1f0);
  std::__cxx11::string::~string(asStack_210);
  std::__cxx11::string::~string(asStack_230);
  std::__cxx11::string::~string(asStack_258);
  pcVar5 = getenv("ZOS_DEBUG_CORE");
  if (pcVar5 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_1a8,"Subscriber Ctor : {}\n");
                    /* try { // try from 001dd018 to 001dd0e3 has its CatchHandler @ 001dd1b8 */
    local_198 = fmt::v9::ptr<>(local_268);
    local_190 = &PTR_s_/home/pi/temp/zos_turing/project_002a8a68;
    log<>::log(auStack_1b0,local_1a8,uStack_1a0,&local_198,&local_190);
  }
  local_170 = _socket_cb;
  local_168 = 0;
  local_160 = local_268;
  std::bind<>((_func_void_void_ptr_ulong *)&local_170,&local_160,
              (_Placeholder *)&std::placeholders::_1,(_Placeholder *)&std::placeholders::_2);
  std::function<>::operator=((function<> *)(local_268 + 0x60),a_Stack_188);
  local_154 = 200;
  setCommLevel((CommLevel *)local_268,SUB81(&local_154,0));
  bVar2 = std::function::operator.cast.to.bool(param_5);
  if (bVar2) {
    std::bind<>(param_5,(_Placeholder *)&std::placeholders::_1);
                    /* try { // try from 001dd0f0 to 001dd0f3 has its CatchHandler @ 001dd198 */
    std::function<>::operator=((function<> *)(local_268 + 0x40),a_Stack_150);
    std::_Bind<>::~_Bind((_Bind<> *)a_Stack_150);
  }
                    /* try { // try from 001dd0fc to 001dd117 has its CatchHandler @ 001dd1b8 */
  pmVar4 = Singleton<>::_<>();
  SubscriberInfo::SubscriberInfo(aSStack_128,(SubscriberInfo *)(local_268 + 0xff28));
                    /* try { // try from 001dd128 to 001dd12b has its CatchHandler @ 001dd1a8 */
  manager::addSub(pmVar4,local_268,aSStack_128);
  SubscriberInfo::~SubscriberInfo(aSStack_128);
  return;
}



/* zos::SubscriberBase::~SubscriberBase() */

void __thiscall zos::SubscriberBase::~SubscriberBase(SubscriberBase *this)

{
  bool bVar1;
  char *pcVar2;
  manager *pmVar3;
  scoped_lock<std::mutex> asStack_150 [8];
  undefined1 auStack_148 [8];
  undefined8 local_140;
  undefined8 uStack_138;
  void *local_130;
  undefined **local_128;
  SubscriberInfo aSStack_120 [288];
  
  pcVar2 = getenv("ZOS_DEBUG_CORE");
  if (pcVar2 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_140,"Subscriber Dtor : {}\n");
    local_130 = fmt::v9::ptr<>(this);
    local_128 = &PTR_s_/home/pi/temp/zos_turing/project_002a8a80;
    log<>::log(auStack_148,local_140,uStack_138,&local_130,&local_128);
  }
  std::atomic<bool>::operator=((atomic<bool> *)this,true);
  std::scoped_lock<std::mutex>::scoped_lock(asStack_150,(mutex *)(this + 8));
  pmVar3 = Singleton<>::_<>();
  SubscriberInfo::SubscriberInfo(aSStack_120,(SubscriberInfo *)(this + 0xff28));
  manager::removeSub(pmVar3,this,aSStack_120);
  SubscriberInfo::~SubscriberInfo(aSStack_120);
  bVar1 = std::atomic::operator.cast.to.bool((atomic *)(this + 0xff20));
  if (bVar1) {
    unsetSocket(SUB81(this,0));
  }
  std::scoped_lock<std::mutex>::~scoped_lock(asStack_150);
  SubscriberInfo::~SubscriberInfo((SubscriberInfo *)(this + 0xff28));
  udp::socket::~socket((socket *)(this + 0x88));
  std::function<>::~function((function<> *)(this + 0x60));
  std::function<>::~function((function<> *)(this + 0x40));
  std::unique_ptr<>::~unique_ptr((unique_ptr<> *)(this + 0x38));
  return;
}



/* zos::SubscriberBase::unsetSocket(bool) */

undefined8 zos::SubscriberBase::unsetSocket(bool param_1)

{
  return 1;
}


/* zos::SubscriberBase::add_task(void const*, unsigned long) */

undefined8 __thiscall
zos::SubscriberBase::add_task(SubscriberBase *this,void *param_1,ulong param_2)

{
  bool bVar1;
  Pool *pPVar2;
  long *plVar3;
  undefined8 uVar4;
  Data aDStack_88 [112];
  scoped_lock<std::mutex> asStack_18 [8];
  future<void> afStack_10 [16];
  
  std::scoped_lock<std::mutex>::scoped_lock(asStack_18,(mutex *)(this + 8));
  bVar1 = std::atomic::operator.cast.to.bool((atomic *)this);
  if (bVar1) {
    uVar4 = 0;
  }
  else {
    bVar1 = std::function::operator.cast.to.bool((function *)(this + 0x40));
    if (bVar1) {
      Data::Data(aDStack_88);
                    /* try { // try from 001e9d90 to 001e9dc7 has its CatchHandler @ 001e9e14 */
      Data::store(aDStack_88,param_1,param_2);
      pPVar2 = Singleton<>::_<>();
      std::move<zos::Data&>(aDStack_88);
      ThreadPool::enqueue<>((function *)pPVar2,(Data *)(this + 0x40));
      std::future<void>::~future(afStack_10);
      Data::~Data(aDStack_88);
    }
    else {
      plVar3 = (long *)std::unique_ptr<>::operator->((unique_ptr<> *)(this + 0x38));
                    /* try { // try from 001e9dfc to 001e9dff has its CatchHandler @ 001e9e24 */
      (**(code **)(*plVar3 + 0x20))(plVar3,param_1,param_2);
    }
    uVar4 = 1;
  }
  std::scoped_lock<std::mutex>::~scoped_lock(asStack_18);
  return uVar4;
}


/* zos::SubscriberBase::_socket_cb(void const*, unsigned long) */

void __thiscall zos::SubscriberBase::_socket_cb(SubscriberBase *this,void *param_1,ulong param_2 )

{
  add_task(this,param_1,param_2);
  return;
}


/* zos::SubscriberBase::setCommLevel(zos::CommLevel const&, bool) */

undefined8 zos::SubscriberBase::setCommLevel(CommLevel *param_1,bool param_2)

{
  int iVar1;
  
  std::atomic<>::operator=((atomic<> *)(param_1 + 0x80),*(undefined4 *)(ulong)param_2);
  iVar1 = std::atomic<>::load(param_1 + 0x80,5);
  if (iVar1 < 0x65) {
    unsetSocket(SUB81(param_1,0));
  }
  else {
    setSocket(SUB81(param_1,0));
  }
  return 1;
}


/* zos::SubscriberBase::setSocket(bool) */

undefined8 zos::SubscriberBase::setSocket(bool param_1)

{
  return 1;
}


/* zos::SubscriberInfo::SubscriberInfo(std::__cxx11::string const&, int, std::__cxx11::string
   const&, int, std::__cxx11::string const&, std::__cxx11::string const&, std::__cxx11::string
   const&, std::__cxx11::string const&, std::__cxx11::string const&, std::__cxx11::string const&,
   void*, zos::InfoFrom) */

void __thiscall
zos::SubscriberInfo::SubscriberInfo
          (SubscriberInfo *this,string *param_1,undefined4 param_2,string *param_3,
          undefined4 param_4,string *param_5,string *param_6,string *param_7,string *param_8,
          string *param_9,string *param_10,undefined8 param_11,undefined4 param_13)

{
  std::__cxx11::string::string((string *)this,param_1);
  *(undefined4 *)(this + 0x20) = param_2;
                    /* try { // try from 001e93dc to 001e93df has its CatchHandler @ 001e94ec */
  std::__cxx11::string::string((string *)(this + 0x28),param_3);
                    /* try { // try from 001e93ec to 001e93ef has its CatchHandler @ 001e94d8 */
  std::__cxx11::string::string((string *)(this + 0x48),param_8);
  *(undefined4 *)(this + 0x68) = param_4;
                    /* try { // try from 001e9408 to 001e940b has its CatchHandler @ 001e94c4 */
  std::__cxx11::string::string((string *)(this + 0x70),param_7);
                    /* try { // try from 001e9418 to 001e941b has its CatchHandler @ 001e94b0 */
  std::__cxx11::string::string((string *)(this + 0x90),param_9);
                    /* try { // try from 001e9428 to 001e942b has its CatchHandler @ 001e949c */
  std::__cxx11::string::string((string *)(this + 0xb0),param_10);
                    /* try { // try from 001e9438 to 001e943b has its CatchHandler @ 001e9488 */
  std::__cxx11::string::string((string *)(this + 0xd0),param_5);
                    /* try { // try from 001e9448 to 001e944b has its CatchHandler @ 001e9474 */
  std::__cxx11::string::string((string *)(this + 0xf0),param_6);
  *(undefined8 *)(this + 0x110) = param_11;
  *(undefined4 *)(this + 0x118) = param_13;
  *(undefined4 *)(this + 0x11c) = 100;
  return;
}


/* zos::SubscriberInfo::SubscriberInfo(zos::SubscriberInfo const&) */

void __thiscall zos::SubscriberInfo::SubscriberInfo(SubscriberInfo *this,SubscriberInfo *param_1 )

{
  std::__cxx11::string::string((string *)this,(string *)param_1);
  *(undefined4 *)(this + 0x20) = *(undefined4 *)(param_1 + 0x20);
                    /* try { // try from 001e97a4 to 001e97a7 has its CatchHandler @ 001e990c */
  std::__cxx11::string::string((string *)(this + 0x28),(string *)(param_1 + 0x28));
                    /* try { // try from 001e97c0 to 001e97c3 has its CatchHandler @ 001e98f8 */
  std::__cxx11::string::string((string *)(this + 0x48),(string *)(param_1 + 0x48));
  *(undefined4 *)(this + 0x68) = *(undefined4 *)(param_1 + 0x68);
                    /* try { // try from 001e97ec to 001e97ef has its CatchHandler @ 001e98e4 */
  std::__cxx11::string::string((string *)(this + 0x70),(string *)(param_1 + 0x70));
                    /* try { // try from 001e9808 to 001e980b has its CatchHandler @ 001e98d0 */
  std::__cxx11::string::string((string *)(this + 0x90),(string *)(param_1 + 0x90));
                    /* try { // try from 001e9824 to 001e9827 has its CatchHandler @ 001e98bc */
  std::__cxx11::string::string((string *)(this + 0xb0),(string *)(param_1 + 0xb0));
                    /* try { // try from 001e9840 to 001e9843 has its CatchHandler @ 001e98a8 */
  std::__cxx11::string::string((string *)(this + 0xd0),(string *)(param_1 + 0xd0));
                    /* try { // try from 001e985c to 001e985f has its CatchHandler @ 001e9894 */
  std::__cxx11::string::string((string *)(this + 0xf0),(string *)(param_1 + 0xf0));
  *(undefined8 *)(this + 0x110) = *(undefined8 *)(param_1 + 0x110);
  *(undefined4 *)(this + 0x118) = *(undefined4 *)(param_1 + 0x118);
  *(undefined4 *)(this + 0x11c) = *(undefined4 *)(param_1 + 0x11c);
  return;
}


/* zos::SubscriberInfo::SubscriberInfo(zos::SubscriberInfo&&) */

void __thiscall zos::SubscriberInfo::SubscriberInfo(SubscriberInfo *this,SubscriberInfo *param_1 )

{
  std::__cxx11::string::string((string *)this,(string *)param_1);
  *(undefined4 *)(this + 0x20) = *(undefined4 *)(param_1 + 0x20);
  std::__cxx11::string::string((string *)(this + 0x28),(string *)(param_1 + 0x28));
  std::__cxx11::string::string((string *)(this + 0x48),(string *)(param_1 + 0x48));
  *(undefined4 *)(this + 0x68) = *(undefined4 *)(param_1 + 0x68);
  std::__cxx11::string::string((string *)(this + 0x70),(string *)(param_1 + 0x70));
  std::__cxx11::string::string((string *)(this + 0x90),(string *)(param_1 + 0x90));
  std::__cxx11::string::string((string *)(this + 0xb0),(string *)(param_1 + 0xb0));
  std::__cxx11::string::string((string *)(this + 0xd0),(string *)(param_1 + 0xd0));
  std::__cxx11::string::string((string *)(this + 0xf0),(string *)(param_1 + 0xf0));
  *(undefined8 *)(this + 0x110) = *(undefined8 *)(param_1 + 0x110);
  *(undefined4 *)(this + 0x118) = *(undefined4 *)(param_1 + 0x118);
  *(undefined4 *)(this + 0x11c) = *(undefined4 *)(param_1 + 0x11c);
  return;
}

/* zos::SubscriberInfo::~SubscriberInfo() */

void __thiscall zos::SubscriberInfo::~SubscriberInfo(SubscriberInfo *this)

{
  std::__cxx11::string::~string((string *)(this + 0xf0));
  std::__cxx11::string::~string((string *)(this + 0xd0));
  std::__cxx11::string::~string((string *)(this + 0xb0));
  std::__cxx11::string::~string((string *)(this + 0x90));
  std::__cxx11::string::~string((string *)(this + 0x70));
  std::__cxx11::string::~string((string *)(this + 0x48));
  std::__cxx11::string::~string((string *)(this + 0x28));
  std::__cxx11::string::~string((string *)this);
  return;
}


/* zos::SubscriberInfo::TEMPNAMEPLACEHOLDERVALUE(zos::SubscriberInfo const&) const */

undefined1 __thiscall zos::SubscriberInfo::operator<(SubscriberInfo *this,SubscriberInfo *param_ 1)

{
  undefined1 uVar1;
  undefined4 uVar2;
  __unspec local_8 [8];
  
  uVar2 = std::operator<=><>((string *)(this + 0x48),(string *)(param_1 + 0x48));
  std::__cmp_cat::__unspec::__unspec(local_8,(__unspec *)0x0);
  uVar1 = std::operator<(uVar2,local_8[0]);
  return uVar1;
}





/* zos::PublisherBase::PublisherBase(std::__cxx11::string const&, std::__cxx11::string const&) */

void __thiscall
zos::PublisherBase::PublisherBase(PublisherBase *this,string *param_1,string *param_2)

{
  string asStack_30 [32];
  allocator aaStack_10 [8];
  allocator *local_8;
  
  local_8 = aaStack_10;
                    /* try { // try from 001dd3e0 to 001dd3e3 has its CatchHandler @ 001dd420 */
  std::__cxx11::string::string<>(asStack_30,"",aaStack_10);
                    /* try { // try from 001dd3f8 to 001dd3fb has its CatchHandler @ 001dd40c */
  PublisherBase(this,asStack_30,param_1,param_2);
  std::__cxx11::string::~string(asStack_30);
  return;
}


/* zos::PublisherBase::PublisherBase(std::__cxx11::string const&, std::__cxx11::string const&,
   std::__cxx11::string const&) */

void __thiscall
zos::PublisherBase::PublisherBase
          (PublisherBase *this,string *param_1,string *param_2,string *param_3)

{
  undefined4 uVar1;
  manager *pmVar2;
  char *pcVar3;
  string asStack_220 [32];
  allocator aaStack_200 [8];
  string asStack_1f8 [32];
  string asStack_1d8 [32];
  string asStack_1b8 [32];
  string asStack_198 [32];
  undefined1 auStack_178 [8];
  undefined8 local_170;
  undefined8 uStack_168;
  void *local_160;
  undefined **local_158;
  PublisherInfo aPStack_150 [328];
  allocator *local_8;
  
  *(undefined ***)this = &PTR_publish_0029dc00;
  *(undefined8 *)(this + 8) = 0;
  *(undefined8 *)(this + 0x10) = 0;
  *(undefined8 *)(this + 0x18) = 0;
  *(undefined8 *)(this + 0x20) = 0;
  *(undefined8 *)(this + 0x28) = 0;
  *(undefined8 *)(this + 0x30) = 0;
  std::set<>::set((set<> *)(this + 8));
  std::shared_mutex::shared_mutex((shared_mutex *)(this + 0x38));
  std::atomic<>::atomic((atomic<> *)(this + 0x70),100);
                    /* try { // try from 001dd4ac to 001dd4af has its CatchHandler @ 001dd748 */
  udp::socket::socket((socket *)(this + 0x78));
  *(undefined8 *)(this + 0xfee0) = 0;
  *(undefined8 *)(this + 0xfee8) = 0;
  *(undefined8 *)(this + 0xfef0) = 0;
  *(undefined8 *)(this + 0xfef8) = 0;
  *(undefined8 *)(this + 0xff00) = 0;
  *(undefined8 *)(this + 0xff08) = 0;
  std::set<>::set((set<> *)(this + 0xfee0));
  std::shared_mutex::shared_mutex((shared_mutex *)(this + 0xff10));
  local_8 = aaStack_200;
                    /* try { // try from 001dd51c to 001dd51f has its CatchHandler @ 001dd6f8 */
  std::__cxx11::string::string<>(asStack_220,"",aaStack_200);
                    /* try { // try from 001dd520 to 001dd537 has its CatchHandler @ 001dd6e4 */
  Singleton<>::_<>();
  manager::uuid[abi:cxx11]();
                    /* try { // try from 001dd538 to 001dd55b has its CatchHandler @ 001dd6d4 */
  pmVar2 = Singleton<>::_<>();
  uVar1 = manager::pid(pmVar2);
  Singleton<>::_<>();
  manager::node_name[abi:cxx11]();
                    /* try { // try from 001dd55c to 001dd573 has its CatchHandler @ 001dd6c4 */
  Singleton<>::_<>();
  manager::node_ns[abi:cxx11]();
                    /* try { // try from 001dd57c to 001dd57f has its CatchHandler @ 001dd6b4 */
  generateUUID[abi:cxx11]();
                    /* try { // try from 001dd5cc to 001dd5cf has its CatchHandler @ 001dd6a4 */
  PublisherInfo::PublisherInfo
            ((PublisherInfo *)(this + 0xff48),asStack_220,asStack_1f8,uVar1,asStack_1d8,asStack_1 b8,
             param_1,asStack_198,param_2,param_3,this,1);
  std::__cxx11::string::~string(asStack_198);
  std::__cxx11::string::~string(asStack_1b8);
  std::__cxx11::string::~string(asStack_1d8);
  std::__cxx11::string::~string(asStack_1f8);
  std::__cxx11::string::~string(asStack_220);
  pcVar3 = getenv("ZOS_DEBUG_CORE");
  if (pcVar3 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_170,"Publisher Ctor : {}\n");
                    /* try { // try from 001dd638 to 001dd683 has its CatchHandler @ 001dd714 */
    local_160 = fmt::v9::ptr<>(this);
    local_158 = &PTR_s_/home/pi/temp/zos_turing/project_002a8a98;
    log<>::log(auStack_178,local_170,uStack_168,&local_160,&local_158);
  }
  pmVar2 = Singleton<>::_<>();
  PublisherInfo::PublisherInfo(aPStack_150,(PublisherInfo *)(this + 0xff48));
                    /* try { // try from 001dd694 to 001dd697 has its CatchHandler @ 001dd704 */
  manager::addPub(pmVar2,this,aPStack_150);
  PublisherInfo::~PublisherInfo(aPStack_150);
  return;
}


/* zos::PublisherBase::~PublisherBase() */

void __thiscall zos::PublisherBase::~PublisherBase(PublisherBase *this)

{
  char *pcVar1;
  manager *pmVar2;
  undefined1 auStack_170 [8];
  undefined8 local_168;
  undefined8 uStack_160;
  void *local_158;
  undefined **local_150;
  PublisherInfo aPStack_148 [328];
  
  *(undefined ***)this = &PTR_publish_0029dc00;
  pcVar1 = getenv("ZOS_DEBUG_CORE");
  if (pcVar1 != (char *)0x0) {
    std::basic_string_view<>::basic_string_view
              ((basic_string_view<> *)&local_168,"Publisher Dtor : {}\n");
    local_158 = fmt::v9::ptr<>(this);
    local_150 = &PTR_s_/home/pi/temp/zos_turing/project_002a8ab0;
    log<>::log(auStack_170,local_168,uStack_160,&local_158,&local_150);
  }
  pmVar2 = Singleton<>::_<>();
  PublisherInfo::PublisherInfo(aPStack_148,(PublisherInfo *)(this + 0xff48));
  manager::removePub(pmVar2,this,aPStack_148);
  PublisherInfo::~PublisherInfo(aPStack_148);
  PublisherInfo::~PublisherInfo((PublisherInfo *)(this + 0xff48));
  std::set<>::~set((set<> *)(this + 0xfee0));
  udp::socket::~socket((socket *)(this + 0x78));
  std::set<>::~set((set<> *)(this + 8));
  return;
}


/* zos::PublisherBase::update2manager() */

void __thiscall zos::PublisherBase::update2manager(PublisherBase *this)

{
  manager *pmVar1;
  PublisherInfo aPStack_148 [328];
  
  pmVar1 = Singleton<>::_<>();
  PublisherInfo::PublisherInfo(aPStack_148,(PublisherInfo *)(this + 0xff48));
                    /* try { // try from 001dd8bc to 001dd8bf has its CatchHandler @ 001dd8cc */
  manager::updatePub(pmVar1,this,aPStack_148);
  PublisherInfo::~PublisherInfo(aPStack_148);
  return;
}


/* zos::PublisherInfo::PublisherInfo(std::__cxx11::string const&, std::__cxx11::string const&, int,
   std::__cxx11::string const&, std::__cxx11::string const&, std::__cxx11::string const&,
   std::__cxx11::string const&, std::__cxx11::string const&, std::__cxx11::string const&, void*,
   zos::InfoFrom) */

void __thiscall
zos::PublisherInfo::PublisherInfo
          (PublisherInfo *this,string *param_1,string *param_2,undefined4 param_3,string *param_4 ,
          string *param_5,string *param_6,string *param_7,string *param_8,string *param_9,
          undefined8 param_10,undefined4 param_12)

{
  undefined1 auStack_18 [8];
  undefined1 auStack_10 [8];
  undefined1 *local_8;
  
  std::__cxx11::string::string((string *)this,param_1);
                    /* try { // try from 001e95d0 to 001e95d3 has its CatchHandler @ 001e9738 */
  std::__cxx11::string::string((string *)(this + 0x20),param_2);
                    /* try { // try from 001e95e0 to 001e95e3 has its CatchHandler @ 001e9724 */
  std::__cxx11::string::string((string *)(this + 0x40),param_7);
  *(undefined4 *)(this + 0x60) = param_3;
                    /* try { // try from 001e95fc to 001e95ff has its CatchHandler @ 001e9710 */
  std::__cxx11::string::string((string *)(this + 0x68),param_6);
                    /* try { // try from 001e960c to 001e960f has its CatchHandler @ 001e96fc */
  std::__cxx11::string::string((string *)(this + 0x88),param_8);
                    /* try { // try from 001e961c to 001e961f has its CatchHandler @ 001e96e8 */
  std::__cxx11::string::string((string *)(this + 0xa8),param_9);
                    /* try { // try from 001e962c to 001e962f has its CatchHandler @ 001e96d4 */
  std::__cxx11::string::string((string *)(this + 200),param_4);
                    /* try { // try from 001e963c to 001e963f has its CatchHandler @ 001e96c0 */
  std::__cxx11::string::string((string *)(this + 0xe8),param_5);
  *(undefined8 *)(this + 0x108) = param_10;
  *(undefined4 *)(this + 0x110) = param_12;
  *(undefined4 *)(this + 0x114) = 100;
  local_8 = auStack_10;
                    /* try { // try from 001e96a0 to 001e96a3 has its CatchHandler @ 001e96ac */
  std::set<>::set(this + 0x118,0,0,auStack_18,auStack_10);
  return;
}


/* zos::PublisherInfo::PublisherInfo(zos::PublisherInfo const&) */

void __thiscall zos::PublisherInfo::PublisherInfo(PublisherInfo *this,PublisherInfo *param_1)

{
  std::__cxx11::string::string((string *)this,(string *)param_1);
                    /* try { // try from 001ea438 to 001ea43b has its CatchHandler @ 001ea5d0 */
  std::__cxx11::string::string((string *)(this + 0x20),(string *)(param_1 + 0x20));
                    /* try { // try from 001ea454 to 001ea457 has its CatchHandler @ 001ea5bc */
  std::__cxx11::string::string((string *)(this + 0x40),(string *)(param_1 + 0x40));
  *(undefined4 *)(this + 0x60) = *(undefined4 *)(param_1 + 0x60);
                    /* try { // try from 001ea480 to 001ea483 has its CatchHandler @ 001ea5a8 */
  std::__cxx11::string::string((string *)(this + 0x68),(string *)(param_1 + 0x68));
                    /* try { // try from 001ea49c to 001ea49f has its CatchHandler @ 001ea594 */
  std::__cxx11::string::string((string *)(this + 0x88),(string *)(param_1 + 0x88));
                    /* try { // try from 001ea4b8 to 001ea4bb has its CatchHandler @ 001ea580 */
  std::__cxx11::string::string((string *)(this + 0xa8),(string *)(param_1 + 0xa8));
                    /* try { // try from 001ea4d4 to 001ea4d7 has its CatchHandler @ 001ea56c */
  std::__cxx11::string::string((string *)(this + 200),(string *)(param_1 + 200));
                    /* try { // try from 001ea4f0 to 001ea4f3 has its CatchHandler @ 001ea558 */
  std::__cxx11::string::string((string *)(this + 0xe8),(string *)(param_1 + 0xe8));
  *(undefined8 *)(this + 0x108) = *(undefined8 *)(param_1 + 0x108);
  *(undefined4 *)(this + 0x110) = *(undefined4 *)(param_1 + 0x110);
  *(undefined4 *)(this + 0x114) = *(undefined4 *)(param_1 + 0x114);
                    /* try { // try from 001ea53c to 001ea53f has its CatchHandler @ 001ea544 */
  std::set<>::set((set<> *)(this + 0x118),(set *)(param_1 + 0x118));
  return;
}


/* zos::PublisherInfo::PublisherInfo(zos::PublisherInfo&&) */

void __thiscall zos::PublisherInfo::PublisherInfo(PublisherInfo *this,PublisherInfo *param_1)

{
  std::__cxx11::string::string((string *)this,(string *)param_1);
  std::__cxx11::string::string((string *)(this + 0x20),(string *)(param_1 + 0x20));
  std::__cxx11::string::string((string *)(this + 0x40),(string *)(param_1 + 0x40));
  *(undefined4 *)(this + 0x60) = *(undefined4 *)(param_1 + 0x60);
  std::__cxx11::string::string((string *)(this + 0x68),(string *)(param_1 + 0x68));
  std::__cxx11::string::string((string *)(this + 0x88),(string *)(param_1 + 0x88));
  std::__cxx11::string::string((string *)(this + 0xa8),(string *)(param_1 + 0xa8));
  std::__cxx11::string::string((string *)(this + 200),(string *)(param_1 + 200));
  std::__cxx11::string::string((string *)(this + 0xe8),(string *)(param_1 + 0xe8));
  *(undefined8 *)(this + 0x108) = *(undefined8 *)(param_1 + 0x108);
  *(undefined4 *)(this + 0x110) = *(undefined4 *)(param_1 + 0x110);
  *(undefined4 *)(this + 0x114) = *(undefined4 *)(param_1 + 0x114);
  std::set<>::set((set<> *)(this + 0x118),(set *)(param_1 + 0x118));
  return;
}


/* zos::PublisherInfo::PublisherInfo() */

void __thiscall zos::PublisherInfo::PublisherInfo(PublisherInfo *this)

{
  allocator aaStack_80 [8];
  allocator aaStack_78 [8];
  allocator aaStack_70 [8];
  allocator aaStack_68 [8];
  allocator aaStack_60 [8];
  allocator aaStack_58 [8];
  allocator aaStack_50 [8];
  allocator aaStack_48 [8];
  allocator *local_40;
  allocator *local_38;
  allocator *local_30;
  allocator *local_28;
  allocator *local_20;
  allocator *local_18;
  allocator *local_10;
  allocator *local_8;
  
  local_8 = aaStack_80;
  std::__cxx11::string::string<>((string *)this,"",aaStack_80);
  local_10 = aaStack_78;
                    /* try { // try from 00233ef4 to 00233ef7 has its CatchHandler @ 002340e0 */
  std::__cxx11::string::string<>((string *)(this + 0x20),"",aaStack_78);
  local_18 = aaStack_70;
                    /* try { // try from 00233f28 to 00233f2b has its CatchHandler @ 002340cc */
  std::__cxx11::string::string<>((string *)(this + 0x40),"",aaStack_70);
  *(undefined4 *)(this + 0x60) = 0;
  local_20 = aaStack_68;
                    /* try { // try from 00233f64 to 00233f67 has its CatchHandler @ 002340b8 */
  std::__cxx11::string::string<>((string *)(this + 0x68),"default",aaStack_68);
  local_28 = aaStack_60;
                    /* try { // try from 00233f98 to 00233f9b has its CatchHandler @ 002340a4 */
  std::__cxx11::string::string<>((string *)(this + 0x88),"",aaStack_60);
  local_30 = aaStack_58;
                    /* try { // try from 00233fcc to 00233fcf has its CatchHandler @ 00234090 */
  std::__cxx11::string::string<>((string *)(this + 0xa8),"",aaStack_58);
  local_38 = aaStack_50;
                    /* try { // try from 00234000 to 00234003 has its CatchHandler @ 0023407c */
  std::__cxx11::string::string<>((string *)(this + 200),"",aaStack_50);
  local_40 = aaStack_48;
                    /* try { // try from 00234034 to 00234037 has its CatchHandler @ 00234068 */
  std::__cxx11::string::string<>((string *)(this + 0xe8),"",aaStack_48);
  *(undefined8 *)(this + 0x108) = 0;
  *(undefined4 *)(this + 0x110) = 0;
  *(undefined4 *)(this + 0x114) = 100;
  std::set<>::set((set<> *)(this + 0x118));
  return;
}


/* zos::PublisherInfo::removeSubs(zos::SubscriberInfo const&) */

undefined8 __thiscall zos::PublisherInfo::removeSubs(PublisherInfo *this,SubscriberInfo *param_1 )

{
  std::set<>::erase((set<> *)(this + 0x118),param_1);
  return 1;
}


/* zos::PublisherInfo::hasSub(zos::SubscriberInfo const&) const */

byte __thiscall zos::PublisherInfo::hasSub(PublisherInfo *this,SubscriberInfo *param_1)

{
  byte bVar1;
  undefined8 local_10;
  undefined8 local_8;
  
  local_10 = std::set<>::find((set<> *)(this + 0x118),param_1);
  local_8 = std::set<>::end((set<> *)(this + 0x118));
  bVar1 = std::operator==((_Rb_tree_const_iterator *)&local_10,(_Rb_tree_const_iterator *)&local _8);
  return bVar1 ^ 1;
}


/* zos::PublisherInfo::addSubs(zos::SubscriberInfo const&) */

undefined8 __thiscall zos::PublisherInfo::addSubs(PublisherInfo *this,SubscriberInfo *param_1)

{
  std::set<>::insert((set<> *)(this + 0x118),param_1);
  return 1;
}


/* zos::PublisherInfo::~PublisherInfo() */

void __thiscall zos::PublisherInfo::~PublisherInfo(PublisherInfo *this)

{
  std::set<>::~set((set<> *)(this + 0x118));
  std::__cxx11::string::~string((string *)(this + 0xe8));
  std::__cxx11::string::~string((string *)(this + 200));
  std::__cxx11::string::~string((string *)(this + 0xa8));
  std::__cxx11::string::~string((string *)(this + 0x88));
  std::__cxx11::string::~string((string *)(this + 0x68));
  std::__cxx11::string::~string((string *)(this + 0x40));
  std::__cxx11::string::~string((string *)(this + 0x20));
  std::__cxx11::string::~string((string *)this);
  return;
}



