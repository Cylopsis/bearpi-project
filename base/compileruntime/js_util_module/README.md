# js_util_module子系统/组件

-   [简介](#简介)
-   [目录](#目录)
-   [说明](#说明)
    -   [接口说明](#接口说明)
    -   [使用说明](#使用说明)

-   [相关仓](#相关仓)

## 简介

UTIL接口用于字符编码TextEncoder、解码TextDecoder、帮助函数HelpFunction、基于Base64的字节编码encode和解码decode、有理数RationalNumber。TextEncoder表示一个文本编码器，接受字符串作为输入，以UTF-8格式进行编码，输出UTF-8字节流。TextDecoder接口表示一个文本解码器，解码器将字节流作为输入，输出stirng字符串。HelpFunction主要是对函数做callback化、promise化以及对错误码进行编写输出，及类字符串的格式化输出。encode接口使用Base64编码方案将指定u8数组中的所有字节编码到新分配的u8数组中或者使用Base64编码方案将指定的字节数组编码为String。decode接口使用Base64编码方案将Base64编码的字符串或输入u8数组解码为新分配的u8数组。rationalnumber有理数主要是对有理数进行比较，获取分子分母等方法。LruBuffer该算法在缓存空间不够的时候，将近期最少使用的数据替换为新数据。该算法源自这样一种访问资源的需求：近期访问的数据，可能在不久的将来会再次访问。于是最少访问的数据就是价值最小的，是最应该踢出缓存空间的数据。Scope接口用于描述一个字段的有效范围。 Scope实例的构造函数用于创建具有指定下限和上限的对象，并要求这些对象必须具有可比性。 


## 目录

```
base/compileruntime/js_util_module/
├── Class:TextEncoder                   # TextEncoder类
│   ├──  new TextEncoder()             	# 创建TextEncoder对象
│   ├──  encode()                      	# encode方法
│   ├──  encoding                     	# encoding属性
│   └──  encodeInto()                   # encodeInto方法
├── Class:TextDecoder                   # TextDecoder类
│   ├──  new TextDecoder()              # 创建TextDecoder对象
│   ├──  decode()             		    # decode方法
│   ├──  encoding                      	# encoding属性
│   ├──  fatal	                     	# fatal属性
│   └──  ignoreBOM                     	# ignoreBOM属性
├── printf()			                # printf方法
├── getErrorString()			        # getErrorString方法
├── callbackWrapper()		            # callbackWrapper方法
├── promiseWrapper()		            # promiseWrapper方法
├── Class:Base64                        # Base64类
│   ├──  new Base64()             	    # 创建Base64对象
│   ├──  encode()                      	# encode方法
│   ├──  encodeToString()               # encodeToString方法
│   └──  decode()                       # decode方法
├── Class:RationalNumber                # RationalNumber类
│   ├──  new RationalNumber()           # 创建RationalNumber对象
│   ├──  CreateRationalFromString()     # CreateRationalFromString方法
│   ├──  CompareTo()                    # CompareTo方法
│   ├──  Equals()                       # Equals方法
│   ├──  Value()                        # Value方法
│   ├──  GetCommonDivisor()             # GetCommonDivisor方法
│   ├──  GetDenominator()               # GetDenominator方法
│   ├──  GetNumerator()                 # GetNumerator方法
│   ├──  IsFinite()                     # IsFinite方法
│   ├──  IsNaN()                        # IsNaN方法
│   ├──  IsZero()                       # IsZero方法
│   └──  ToString()                     # ToString方法
├── Class:LruBuffer                 	# LruBuffer类
│   ├──  new LruBuffer()                # 创建LruBuffer对象
│   ├──  updateCapacity()               # updateCapacity方法
│   ├──  toString()                     # toString方法
│   ├──  values()                       # values方法
│   ├──  size()                         # size方法
│   ├──  capacity()                     # capacity方法
│   ├──  clear()                        # clear方法
│   ├──  getCreateCount()               # getCreateCount方法
│   ├──  getMissCount()                 # getMissCount方法
│   ├──  getRemovalCount()              # getRemovalCount方法
│   ├──  getMatchCount()                # getMatchCount方法
│   ├──  getPutCount()                  # getPutCount方法
│   ├──  isEmpty()                      # isEmpty方法
│   ├──  get()                          # get方法
│   ├──  put()                          # put方法
│   ├──  keys()                         # keys方法
│   ├──  remove()                       # remove方法
│   ├──  afterRemoval()                 # afterRemoval方法
│   ├──  contains()                		# contains方法
│   ├──  createDefault()                # createDefault方法
│   ├──  entries()               		# entries方法
│   └──  [Symbol.iterator]()            # Symboliterator方法
└── Class:Scope                         # Scope类
    ├── constructor()                   # 创建Scope对象
    ├── toString()                      # toString方法
    ├── intersect()                     # intersect方法
    ├── intersect()                     # intersect方法
    ├── getUpper()                      # getUpper方法
    ├── getLower()                      # getLower方法
    ├── expand()                        # expand方法 
    ├── expand()                        # expand方法
    ├── expand()                        # expand法
    ├── contains()                      # contains方法
    ├── contains()                      # contains方法
    └── clamp()                         # clamp方法
```

## 说明

### 接口说明


| 接口名 | 说明 |
| -------- | -------- |
| readonly encoding : string | 获取编码的格式，只支持UTF-8。 |
| encode(input : string) : Uint8Array | 输入stirng字符串，编码并输出UTF-8字节流。 |
| encodeInto(input : string, dest : Uint8Array) : {read : number, written : number} | 输入stirng字符串，dest表示编码后存放位置，返回一个对象，read表示已经编码的字符的个数，written表示已编码字符所占字节的大小。 |
| constructor(encoding? : string, options? : {fatal? : boolean, ignoreBOM? : boolean}) | 构造函数，第一个参数encoding表示解码的格式。第二个参数表示一些属性。属性中fatal表示是否抛出异常，ignoreBOM表示是否忽略bom标志。 |
| readonly encoding : string | 获取设置的解码格式。 |
| readonly fatal : boolean | 获取抛出异常的设置 |
| readonly ignoreBOM : boolean | 获取是否忽略bom标志的设置 |
| decode(input : ArrayBuffer | 输入要解码的数据，解出对应的string字符串。第一个参数input表示要解码的数据，第二个参数options表示一个bool标志，表示将跟随附加数据，默认为false。 |
| function printf(format: string, ...args: Object[]): string | printf()方法使用第一个参数作为格式字符串（其可以包含零个或多个格式说明符）来返回格式化的字符串。 |
| function getErrorString(errno: number): string | getErrorString()方法使用一个系统的错误数字作为参数，用来返回系统的错误信息。 |
| function callbackWrapper(original: Function): (err: Object, value: Object) => void | 参数为一个采用 async 函数（或返回 Promise 的函数）并返回遵循错误优先回调风格的函数，即将 (err, value) => ... 回调作为最后一个参数。 在回调中，第一个参数将是拒绝原因（如果 Promise 已解决，则为 null），第二个参数将是已解决的值。 |
| function promiseWrapper(original: (err: Object, value: Object) => void): Object |     参数为采用遵循常见的错误优先的回调风格的函数（也就是将 (err, value) => ... 回调作为最后一个参数），并返回一个返回 promise 的版本。 |
| encode(src: Uint8Array): Uint8Array; | 使用Base64编码方案将指定u8数组中的所有字节编码到新分配的u8数组中。 |
| encodeToString(src: Uint8Array): string; | 使用Base64编码方案将指定的字节数组编码为String。 |
| decode(src: Uint8Array / string): Uint8Array; | 使用Base64编码方案将Base64编码的字符串或输入u8数组解码为新分配的u8数组。 |
| CreateRationalFromString(src: string): Rational | 基于给定的字符串创建一个RationalNumber对象 |
| CompareTo(src: RationalNumber): number | 将当前的RationalNumber对象与给定的对象进行比较 |
| Equals(src: object): number | 检查给定对象是否与当前 RationalNumber 对象相同 |
| Value(): number | 将当前的RationalNumber对象进行取整数值或者浮点数值 |
| GetCommonDivisor(arg1: int, arg2: int,): number | 获得两个指定数的最大公约数 |
| GetDenominator(): number | 获取当前的RationalNumber对象的分母 |
| GetNumerator(): number | 获取当前的RationalNumber对象的分子 |
| IsFinite(): bool | 检查当前的RationalNumber对象是有限的 |
| IsNaN(): bool | 检查当前RationalNumber对象是否表示非数字(NaN)值 |
| IsZero(): bool | 检查当前RationalNumber对象是否表示零值 |
| ToString(): string | 获取当前RationalNumber对象的字符串表示形式 |
| updateCapacity(newCapacity:number):void | 将缓冲区容量更新为指定容量，如果 newCapacity 小于或等于 0，则抛出此异常 |
| toString():string | 返回对象的字符串表示形式，输出对象的字符串表示。 |
| values():V[ ] | 获取当前缓冲区中所有值的列表，输出按升序返回当前缓冲区中所有值的列表，从最近访问到最近最少访问 |
| size():number | 获取当前缓冲区中值的总数，输出返回当前缓冲区中值的总数 |
| capacity():number | 获取当前缓冲区的容量，输出返回当前缓冲区的容量 |
| clear():void | 从当前缓冲区清除键值对，清除键值对后，调用afterRemoval()方法依次对其执行后续操作 |
| getCreateCount():number | 获取createDefault()返回值的次数,输出返回createDefault()返回值的次数 |
| getMissCount():number | 获取查询值不匹配的次数，输出返回查询值不匹配的次数 |
| getRemovalCount():number | 获取从缓冲区中逐出值的次数，输出从缓冲区中驱逐的次数 |
| getMatchCount​():number | 获取查询值匹配成功的次数，输出返回查询值匹配成功的次数 |
| getPutCount():number | 获取将值添加到缓冲区的次数，输出返回将值添加到缓冲区的次数 |
| isEmpty():boolean | 检查当前缓冲区是否为空，输出如果当前缓冲区不包含任何值，则返回 true |
| get(k:key):V / undefined | 表示要查询的键，输出如果指定的键存在于缓冲区中，则返回与键关联的值；否则返回 undefined |
| put(K key, V value):V | 将键值对添加到缓冲区，输出与添加的键关联的值；如果要添加的键已经存在，则返回原始值，如果键或值为空，则抛出此异常 |
| keys():K[ ] | 获取当前缓冲区中值的键列表，输出返回从最近访问到最近最少访问排序的键列表 |
| remove​(k:key):V / undefined |  从当前缓冲区中删除指定的键及其关联的值 |
| afterRemoval(boolean isEvict, K key, V value, V newValue):void | 删除值后执行后续操作 |
| contains(k:key):boolean | 检查当前缓冲区是否包含指定的键，输出如果缓冲区包含指定的键，则返回 true |
| createDefault(k:key):V | 如果未计算特定键的值，则执行后续操作，参数表示丢失的键,输出返回与键关联的值 |
| entries() : [K,V] | 允许迭代包含在这个对象中的所有键/值对。每对的键和值都是对象 |
| Symbol.iterator():[K,V] | 返回以键值对得形式得一个二维数组 |
| constructor(lowerObj: ScopeType, upperObj: ScopeType) | 创建并返回一个Scope对象，用于创建指定下限和上限的作用域实例的构造函数。 |
| toString():string | 该字符串化方法返回一个包含当前范围的字符串表示形式。 |
| intersect(range: Scope): Scope | 该方法返回一个给定范围和当前范围的交集。 |
| intersect(lowerObj: ScopeType, upperObj: ScopeType): Scope | 传入给定范围的上限和下限，返回当前范围与给定下限和上限指定的范围的交集。 |
| getUpper(): ScopeType | 获取当前范围的上界，返回一个ScopeType类型的值。 |
| getLower(): ScopeType | 获取当前范围的下界，返回一个ScopeType类型的值。 |
| expand(lowerObj: ScopeType, upperObj:  ScopeType): Scope | 该方法创建并返回包括当前范围和给定下限和上限的并集。 |
| expand(range: Scope): Scope | 该方法创建并返回包括当前范围和给定范围的并集。 |
| expand(value: ScopeType): Scope | 该方法创建并返回包括当前范围和给定值的并集。 |
| contains(value: ScopeType): boolean | 检查给定value是否包含在当前范围内。有则返回true，否则返回false。 |
| contains(range: Scope): boolean | 检查给定range是否在当前范围内。如果在则返回true，否则返回false。 |
| clamp(value: ScopeType): ScopeType | 将给定value限定到当前范围内，如果传入的value小于下限，则返回lowerObj；如果大于上限值则返回upperObj；如果在当前范围内，则返回value。 |

printf中每个说明符都替换为来自相应参数的转换后的值。 支持的说明符有:
| 式样化字符 | 式样要求 |
| -------- | -------- |
|    %s: | String 将用于转换除 BigInt、Object 和 -0 之外的所有值。|
|    %d: |Number 将用于转换除 BigInt 和 Symbol 之外的所有值。|
|    %i:  |parseInt(value, 10) 用于除 BigInt 和 Symbol 之外的所有值。|
|    %f:  |parseFloat(value) 用于除 Symbol 之外的所有值。|
|    %j:  |JSON。 如果参数包含循环引用，则替换为字符串 '[Circular]'。|
|    %o:  |Object. 具有通用 JavaScript 对象格式的对象的字符串表示形式。类似于具有选项 { showHidden: true, showProxy: true } 的 util.inspect()。这将显示完整的对象，包括不可枚举的属性和代理。|
|    %O:  |Object. 具有通用 JavaScript 对象格式的对象的字符串表示形式。类似于没有选项的 util.inspect()。 这将显示完整的对象，但不包括不可枚举的属性和代理。|
|    %c:  | 此说明符被忽略，将跳过任何传入的 CSS。|
|    %%:  |单个百分号 ('%')。 这不消耗待式样化参数。|

### 使用说明

各接口使用方法如下：

1.readonly encoding()

```
import util from '@ohos.util'
var textEncoder = new util.TextEncoder();
var getEncoding = textEncoder.encoding();
```
2.encode()
```
import util from '@ohos.util'
var textEncoder = new util.TextEncoder();
var result = textEncoder.encode('abc');
```
3.encodeInto()
```
import util from '@ohos.util'
var textEncoder = new util.TextEncoder();
var obj = textEncoder.encodeInto('abc', dest);
```
4.textDecoder()
```
import util from '@ohos.util'
var textDecoder = new util.textDecoder("utf-16be", {fatal : ture, ignoreBOM : false});
```
5.readonly encoding()
```
import util from '@ohos.util'
var textDecoder = new util.textDecoder("utf-16be", {fatal : ture, ignoreBOM : false});
var getEncoding = textDecoder.encoding();
```
6.readonly fatal()
```
import util from '@ohos.util'
var textDecoder = new util.textDecoder("utf-16be", {fatal : ture, ignoreBOM : false});
var fatalStr = textDecoder.fatal();
```
7.readonly ignoreBOM()
```
import util from '@ohos.util'
var textDecoder = new util.textDecoder("utf-16be", {fatal : ture, ignoreBOM : false});
var ignoreBom = textDecoder.ignoreBOM();
```
8.decode()
```
import util from '@ohos.util'
var textDecoder = new util.textDecoder("utf-16be", {fatal : ture, ignoreBOM : false});
var result = textDecoder.decode(input, {stream : true});
```
9.printf()
```
import util from '@ohos.util'
var format = "%%%o%%%i%s";
var value =  function aa(){};
var value1 = 1.5;
var value2 = "qwer";
var result = util.printf(format,value,value1,value2);
```
10.getErrorString()
```
import util from '@ohos.util'
var errnum = 13;
var result = util.getErrorString(errnum);
```
11.callbackWrapper()
```
import util from '@ohos.util'
async function promiseFn() {
    return Promise.resolve('value');
};
var cb = util.callbackWrapper(promiseFn);
cb((err, ret) => {
    expect(err).strictEqual(null);
    expect(ret).strictEqual('value');
})
```
12.promiseWrapper()
```
import util from '@ohos.util'
function aysnFun(str1, str2, callback) {
    if (typeof str1 === 'string' && typeof str1 === 'string') {
        callback(null, str1 + str2);
    } else {
        callback('type err');
    }
}
let newPromiseObj = util.promiseWrapper(aysnFun)("Hello", 'World');
newPromiseObj.then(res => {
    expect(res).strictEqual('HelloWorld');
})
```
13.encode()
```
import util from '@ohos.util'
var that = new util.Base64();
var array = new Uint8Array([115,49,51]);
var num = 0;
var result = that.encode(array,num);
```
14.encodeToString()
```
import util from '@ohos.util'
var that = new util.Base64();
var array = new Uint8Array([115,49,51]);
var num = 0;
var result = that.encodeToString(array,num);
```
15.decode()
```
import util from '@ohos.util'
var that = new util.Base64()
var buff = 'czEz';
var num = 0;
var result = that.decode(buff,num);
```
16.createRationalFromString()
```
import util from '@ohos.util'
var pro = new util.RationalNumber(0, 0);
var res = pro.createRationalFromString("-1:2");
var result1 = res.value();
```
17.compareTo()
```
import util from '@ohos.util'
var pro = new util.RationalNumber(2, 1);
var proc = new util.RationalNumber(3, 4);
var res = pro.compareTo(proc);
```
18.equals()
```
import util from '@ohos.util'
var pro = new util.RationalNumber(2, 1);
var proc = new util.RationalNumber(3, 4);
var res = pro.equals(proc);
```
19.value()
```
import util from '@ohos.util'
var pro = new util.RationalNumber(2, 1);
var res = pro.value();
```
20.getCommonDivisor()
```
import util from '@ohos.util'
var pro = new util.RationalNumber(0, 0);
var res = pro.getCommonDivisor(4, 8);
```
21.getDenominator()
```
import util from '@ohos.util'
var pro = new util.RationalNumber(2, 1);
var res = pro.getDenominator();
```
22.getNumerator()
```
import util from '@ohos.util'
var pro = new util.RationalNumber(-2, 1);
var res = pro.getNumerator();
```
23.isFinite()
```
import util from '@ohos.util'
var pro = new util.RationalNumber(-2, 1);
var res = pro.isFinite();
```
24.isNaN()
```
import util from '@ohos.util'
var pro = new util.RationalNumber(-2, 1);
var res = pro.isNaN();
```
25.isZero()
```
import util from '@ohos.util'
var pro = new util.RationalNumber(-2, 1);
var res = pro.isZero();

```
26.toString()
```
import util from '@ohos.util'
var pro = new util.RationalNumber(-2, 1);
var res = pro.toString();

```
27.updateCapacity() 
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.updateCapacity(100);
```
28.toString()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
pro.get(2);
pro.remove(20);
var temp = pro.toString();
```
29.values()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
pro.put(2,"anhu");
pro.put("afaf","grfb");
var temp = pro.values();
```
30.size()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
pro.put(1,8);
var temp = pro.size();
```
31.capacity()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
var temp = pro.capacity();
```
32.clear() 
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
pro.clear();
```
33.getCreatCount()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(1,8);
var temp = pro.getCreatCount();
```
34.getMissCount()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
pro.get(2)
var temp = pro.getMissCount();
```
35.getRemovalCount()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
pro.updateCapacity(2);
pro.put(50,22);
var temp = pro.getRemovalCount();
```
36.getMatchCount()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
pro.get(2);
var temp = pro.getMatchCount();
```
37.getPutCount()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
var temp = pro.getPutCount();
```
38.isEmpty()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
var temp = pro.isEmpty();
```
39.get()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
var temp = pro.get(2);
```
40.put()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
var temp = pro.put(2,10);
```
41.keys()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
 pro.put(2,10);
var temp = pro.keys();
```
42.remove()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
var temp = pro.remove(20);
```
43.contains()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
var temp = pro.contains(20);
```
44.createDefault()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
var temp = pro.createDefault(50);
```
45.entries()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
var temp = pro.entries();
```
46.[Symbol.iterator]()
```
import util from '@ohos.util'
var pro = new util.LruBuffer();
pro.put(2,10);
var temp = aa[symbol.iterator]();
```
Scope接口中构造新类，实现compareTo方法。

```
class Temperature {
    constructor(value) {
        this._temp = value;
    }
    compareTo(value) {
        return this._temp >= value.getTemp();
    }
    getTemp() {
        return this._temp;
    }
    toString() {
        return this._temp.toString();
    }
}
```

47.constructor()

```
var tempLower = new Temperature(30);
var tempUpper = new Temperature(40);
var range = new Scope(tempLower, tempUpper);
```

48.toString()

```
var tempLower = new Temperature(30);
var tempUpper = new Temperature(40);
var range = new Scope(tempLower, tempUpper);
range.toString() // => [30,40]
```

49.intersect()

```
var tempLower = new Temperature(30);
var tempUpper = new Temperature(40);
var range = new Scope(tempLower, tempUpper);
var tempMiDF = new Temperature(35);
var tempMidS = new Temperature(39);
var rangeFir = new Scope(tempMiDF, tempMidS);
range.intersect(rangeFir)  // => [35,39]
```

50.intersect()

```
var tempLower = new Temperature(30);
var tempUpper = new Temperature(40);
var tempMiDF = new Temperature(35);
var tempMidS = new Temperature(39);
var range = new Scope(tempLower, tempUpper);
range.intersect(tempMiDF, tempMidS)  // => [35,39]
```

51.getUpper()

```
var tempLower = new Temperature(30);
var tempUpper = new Temperature(40);
var range = new Scope(tempLower, tempUpper);
range.getUpper() // => 40
```

52.getLower()

```
var tempLower = new Temperature(30);
var tempUpper = new Temperature(40);
var range = new Scope(tempLower, tempUpper);
range.getLower() // => 30
```

53.expand()

```
var tempLower = new Temperature(30);
var tempUpper = new Temperature(40);
var tempMiDF = new Temperature(35);
var tempMidS = new Temperature(39);
var range = new Scope(tempLower, tempUpper);
range.expand(tempMiDF, tempMidS)  // => [30,40]
```

54.expand()

```
var tempLower = new Temperature(30);
var tempUpper = new Temperature(40);
var tempMiDF = new Temperature(35);
var tempMidS = new Temperature(39);
var range = new Scope(tempLower, tempUpper);
var rangeFir = new Scope(tempMiDF, tempMidS);
range.expand(rangeFir) // => [30,40]
```

55.expand()

```
var tempLower = new Temperature(30);
var tempUpper = new Temperature(40);
var tempMiDF = new Temperature(35);
var range = new Scope(tempLower, tempUpper);
range.expand(tempMiDF)  // => [30,40]
```

56.contains()

```
var tempLower = new Temperature(30);
var tempUpper = new Temperature(40);
var tempMiDF = new Temperature(35);
var range = new Scope(tempLower, tempUpper);
range.contains(tempMiDF) // => true
```

57.contains()

```
var tempLower = new Temperature(30);
var tempUpper = new Temperature(40);
var range = new Scope(tempLower, tempUpper);
var tempLess = new Temperature(20);
var tempMore = new Temperature(45);
var rangeSec = new Scope(tempLess, tempMore);
range.contains(rangeSec) // => true
```

58.clamp()

```
var tempLower = new Temperature(30);
var tempUpper = new Temperature(40);
var tempMiDF = new Temperature(35);
var range = new Scope(tempLower, tempUpper);
range.clamp(tempMiDF) // => 35
```



## 相关仓

[js_util_module子系统](https://gitee.com/OHOS_STD/js_util_module)

[base/compileruntime/js_util_module/](base/compileruntime/js_util_module-readme.md)
