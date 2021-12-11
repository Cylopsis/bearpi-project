# js_api_module子系统/组件

-   [简介](#简介)
-   [目录](#目录)
-   [说明](#说明)
    -   [接口说明](#接口说明)
    -   [使用说明](#使用说明)

-   [相关仓](#相关仓)

## 简介

URL接口用于解析，构造，规范化和编码 URLs。 URL的构造函数创建新的URL对象。 以便对URL的已解析组成部分或对URL进行更改。URLSearchParams 接口定义了一些实用的方法来处理 URL 的查询字符串。

URI表示统一资源标识符引用。

xml表示指可扩展标记语言。

## 目录

```
base/compileruntime/js_api_module/
├── Class:URL                                  # URL类
│   ├── new URL(input[, base])                 # 创建URL对象
│   ├── hash                                   # hash属性
│   ├── host                                   # host属性
│   ├── hostname                               # hostname属性
│   ├── href                                   # href属性
│   ├── origin                                 # origin属性
│   ├── password                               # password属性
│   ├── pathname                               # pathname属性
│   ├── port                                   # port属性
│   ├── protocol                               # protocol属性
│   ├── search                                 # search属性
│   ├── searchParams                           # searchParams属性
│   ├── username                               # username属性
│   ├── toString()                             # toString方法
│   └── toJSON()                               # toJSON方法
├── Class: URLSearchParams                 # URLSearchParams类
│   ├── new URLSearchParams()                  # 创建URLSearchParams对象
│   ├── new URLSearchParams(string)            # 创建URLSearchParams对象
│   ├── new URLSearchParams(obj)               # 创建URLSearchParams对象
│   ├── new URLSearchParams(iterable)          # 创建URLSearchParams对象
│   ├── append(name, value)                    # append方法
│   ├── delete(name)                           # delete方法
│   ├── entries()                              # entries方法
│   ├── forEach(fn[, thisArg])                 # forEach方法
│   ├── get(name)                              # get方法
│   ├── getAll(name)                           # getAll方法
│   ├── has(name)                              # has方法
│   ├── keys()                                 # keys方法
│   ├── set(name, value)                       # set方法
│   ├── sort()                                 # sort方法
│   ├── toString()                             # toString方法
│   ├── values()                               # values方法
│   └── urlSearchParams[Symbol.iterator]()     # 创建URLSearchParams对象 
├── Class:URI                              # URI类
│   ├── URI​(String str)                        # 创建URI对象
│   ├── scheme                                 # scheme属性
│   ├── authority                              # authority属性
│   ├── ssp                                    # ssp属性
│   ├── userinfo                               # userinfo属性
│   ├── host                                   # host属性
│   ├── port                                   # port属性
│   ├── query                                  # query属性
│   ├── fragment                               # fragment属性
│   ├── path                                   # path属性
│   ├── equals(Object ob)                      # equals方法
│   ├── normalize​()                            # normalize方法
│   ├── isAbsolute​()                           # isAbsolute方法
│   ├── normalize​()                            # normalize方法
│   └── toString()                             # toString方法
└── Class:ConvertXml                       # ConvertXml类
	├── ConvertXml()                           # 创建ConvertXml类对象
	└── convert(String xml, Object options)    # convert方法	
```

## 说明

### 接口说明


| 接口名 | 说明                                                         |
| -------- | -------- | 
//URL
| URL(url: string,base?:string I URL) | 创建并返回一个URL对象，该URL对象引用使用绝对URL字符串，相对URL字符串和基本URL字符串指定的URL。 |
| tostring():string | 该字符串化方法返回一个包含完整 URL 的 USVString。它的作用等同于只读的 URL.href。 |
| toJSON():string | 该方法返回一个USVString，其中包含一个序列化的URL版本。 |
| new URLSearchParams() | URLSearchParams() 构造器无入参，该方法创建并返回一个新的URLSearchParams 对象。 开头的'?' 字符会被忽略。 |
| new URLSearchParams(string) | URLSearchParams(string) 构造器的入参为string数据类型，该方法创建并返回一个新的URLSearchParams 对象。 开头的'?' 字符会被忽略。 |
| new URLSearchParams(obj) | URLSearchParams(obj) 构造器的入参为obj数据类型，该方法创建并返回一个新的URLSearchParams 对象。 开头的'?' 字符会被忽略。 |
| new URLSearchParams(iterable) | URLSearchParams(iterable) 构造器的入参为iterable数据类型，该方法创建并返回一个新的URLSearchParams 对象。 开头的'?' 字符会被忽略。 |
| has(name: string): boolean | 检索searchParams对象中是否含有name。有则返回ture，否则返回false。 |
| set(name: string, value string): void |  检索searchParams对象中是否含有key为name的键值对。没有的话则添加该键值对，有的话则修改对象中第一个key所对应的value，并删除键为name的其余键值对。 |
| sort(): void | 根据键的Unicode代码点，对包含在此对象中的所有键/值对进行排序，并返回undefined。 |
| toString(): string | 根据searchParams对象,返回适用在URL中的查询字符串。 |
| keys(): iterableIterator<string> | 返回一个iterator，遍历器允许遍历对象中包含的所有key值。 |
| values(): iterableIterator<string> | 返回一个iterator，遍历器允许遍历对象中包含的所有value值。 |
| append(name: string, value: string): void | 在searchParams对象中插入name, value键值对。 |
| delete(name: string): void | 遍历searchParams对象，查找所有的name,删除对应的键值对。 |
| get(name: string): string | 检索searchParams对象中第一个name,返回name键对应的值。 |
| getAll(name: string): string[] | 检索searchParams对象中所有name,返回name键对应的所有值。 |
| entries(): iterableIterator<[string, string]> | 返回一个iterator，允许遍历searchParams对象中包含的所有键/值对。 |
| forEach(): void | 通过回调函数来遍历URLSearchParams实例对象上的键值对。 |
| urlSearchParams[Symbol.iterator] () | 返回查询字符串中每个名称-值对的ES6迭代器。迭代器的每个项都是一个JavaScript数组。 |

//URI
| URI​(String str) | 通过解析给定入参（String str）来构造URI。此构造函数严格按照RFC 2396附录A中的语法规定解析给定字符串。 |
| scheme​ | 返回此 URI 的scheme部分，如果scheme未定义，则返回 null |
| authority​ | 返回此 URI 的解码authority部分，如果authority未定义，则返回 null。 |
| ssp​ |  返回此 URI 的解码scheme-specific部分。 |
| userinfo​ | 返回此 URI 的解码userinfo部分。包含passworld和username。 |
| host​ | 返回此 URI 的host部分，如果host未定义，则返回 null。 |
| port​ | 返回此 URI 的port部分，如果port未定义，则返回 -1。URI 的port组件（如果已定义）是一个非负整数。 |
| query​ | 返回此 URI 的query部分，如果query未定义，则返回 null。 |
| fragment​ | 返回此 URI 的解码fragment组件，如果fragment未定义，则返回 null。
| path​ | 返回此 URI 的解码path组件，如果path未定义，则返回 null。 |
| equals(Object ob) | 测试此 URI 是否与另一个对象相等。如果给定的对象不是 URI，则此方法立即返回 false。 |
| normalize​() | 规范化这个 URI 的路径。如果这个 URI 的path不规范，将规范后构造一个新 URI对象返回。 |
| isAbsolute​() | 判断这个 URI 是否是绝对的。当且仅当它具有scheme部分时，URI 是绝对的，返回值为true，否则返回值为false。 |

//ConvertXml
| ConvertXml() | 用于构造ConvertXml类对象的构造函数。此构造函数无需传入参数。 |
| convert(String xml, Object options)  | 返回按选项要求转化xml字符串的JavaScrip对象。 |

### 使用说明

各接口使用方法如下：

1.URL

1、new URL(url: string,base?:string|URL)
```
let b = new URL('https://developer.mozilla.org'); // => 'https://developer.mozilla.org/'

let a = new URL( 'sca/./path/path/../scasa/text', 'http://www.example.com');
// => 'http://www.example.com/sca/path/scasa/text'
```
2、tostring():string
```
const url = new URL('http://10.0xFF.O400.235:8080/directory/file?query#fragment');
url.toString() // => 'http://10.0xff.o400.235:8080/directory/file?query#fragment'
   
const url = new URL("http://[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]:80/index.html");
url.toString() // => 'http://[fedc:ba98:7654:3210:fedc:ba98:7654:3210]/index.html'

const url = new URL("http://username:password@host:8080/directory/file?query#fragment");
url.toString() // => 'http://username:password@host:8080/directory/file?query#fragment'
```
3、toJSON():string
```
const url = new URL("https://developer.mozilla.org/en-US/docs/Web/API/URL/toString");
url.toJSON(); // =>  'https://developer.mozilla.org/en-US/docs/Web/API/URL/toString'
```
4、new URLSearchParams()
```
let params = new URLSearchParams('foo=1&bar=2');
```
5、new URLSearchParams(string)
```
params = new URLSearchParams('user=abc&query=xyz');
console.log(params.get('user'));
// Prints 'abc'
```
6、new URLSearchParams(obj)
```
const params = new URLSearchParams({
    user: 'abc',
    query: ['first', 'second']
});
console.log(params.getAll('query'));
// Prints [ 'first,second' ]
```
7、new URLSearchParams(iterable)
```
let params;

// Using an array
params = new URLSearchParams([
    ['user', 'abc'],
    ['query', 'first'],
    ['query', 'second'],
]);
console.log(params.toString());
// Prints 'user = abc & query = first&query = second'
```
8、has(name: string): boolean
```
console.log(params.has('bar')); // =>ture
```
9、set(name: string, value string): void
```
params.set('baz', 3);
```
10、sort(): void
```
params .sort();
```
11、toString(): string
```
console.log(params .toString()); // =>bar=2&baz=3&foo=1'
```
12、keys(): iterableIterator<string>
```
for(var key of params.keys()) {
  console.log(key);
} // =>bar  baz  foo
```
13、values(): iterableIterator<string>
```
for(var value of params.values()) {
  console.log(value);
} // =>2  3  1
```
14、append(name: string, value: string): void
```
params.append('foo', 3); // =>bar=2&baz=3&foo=1&foo=3
```
15、delete(name: string): void
```
params.delete('baz'); // => bar=2&foo=1&foo=3
```
16、get(name: string): string
```
params.get('foo'); // => 1
```
17、getAll(name: string): string[]
```
params.getAll('foo'); // =>[ '1', '3' ]
```
18、entries(): iterableIterator<[string, string]>
```
for(var pair of searchParams.entries()) {
   console.log(pair[0]+ ', '+ pair[1]);
} // => bar, 2   foo, 1  foo, 3
```
19、forEach(): void
```
url.searchParams.forEach((value, name, searchParams) => {
  console.log(name, value, url.searchParams === searchParams);
});
// => foo 1 true
// => bar 2 true
```
20、urlSearchParams[Symbol.iterator] ()
```
const params = new URLSearchParams('foo=bar&xyz=baz');
for (const [name, value] of params) {
    console.log(name, value);
}
// Prints:
// foo bar
// xyz ba
```

2.URI

1、URI​(String str)
```
let gaogao = new Uri.URI('http://gg:gaogao@www.baidu.com:99/path/path?query#fagment');
```
2、scheme
```
let gaogao = new Uri.URI('http://gg:gaogao@www.baidu.com:99/path/path?query#fagment');
gaogao.scheme        // => "http";
```
3、authority
```
let gaogao = new Uri.URI('http://gg:gaogao@www.baidu.com:99/path/path?query#fagment');
gaogao.authority     // => "gg:gaogao@www.baidu.com:99";
```
4、ssp
```
let gaogao = new Uri.URI('http://gg:gaogao@www.baidu.com:99/path/path?query#fagment');
gaogao.ssp "         // => gg:gaogao@www.baidu.com:99/path/path?query";
```
5、userinfo
```
let gaogao = new Uri.URI('http://gg:gaogao@www.baidu.com:99/path/path?query#fagment');
gaogao.userinfo      // => "gg:gaogao";
```
6、host
```
let gaogao = new Uri.URI('http://gg:gaogao@www.baidu.com:99/path/path?query#fagment');
gaogao.host          // => "www.baidu.com";
```
7、port
```
let gaogao = new Uri.URI('http://gg:gaogao@www.baidu.com:99/path/path?query#fagment');
gaogao.port          // => "99";
```
8、query
```
let gaogao = new Uri.URI('http://gg:gaogao@www.baidu.com:99/path/path?query#fagment');
gaogao.query         // => "query";
```
9、fragment
```
let gaogao = new Uri.URI('http://gg:gaogao@www.baidu.com:99/path/path?query#fagment');
gaogao.fragment      // => "fagment";
```
10、path
```
let gaogao = new Uri.URI('http://gg:gaogao@www.baidu.com:99/path/path?query#fagment');
gaogao.path          // => "/path/path";
```
11、equals(Object ob)
```
let gaogao = new Uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment');
let gaogao1 = gaogao;
let res = gaogao.equals(gaogao1);
console.log(res);      // => true;
```
12、normalize​()
```
let gaogao = new Uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path/66./../././mm/.././path1?query#fagment');
let res = gaogao.normalize();
console.log(res.path);        // => "/path/path1"
console.log(res.toString());  // => "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path/path1?query#fagment"
```
13、isAbsolute​()
```
let gaogao = new Uri.URI('f/tp://username:password@www.baidu.com:88/path?query#fagment');
let res = gaogao.isAbsolute();
console.log(res);              //=> false;
```
14、toString()
```
let gaogao = new Uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path/.././../aa/bb/cc?query#fagment');
let res = gaogao.toString();
console.log(res.toString());     // => 'http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path/.././../aa/bb/cc?query#fagment';
```

3.ConvertXml

1、ConvertXml()
```
var convertml = new convertXml.ConvertXml();
```
2、convert(String xml, Object options)
```
var result = convertml.convert(xml, {compact: false, spaces: 4});
```

## 相关仓
[js_api_module子系统](https://gitee.com/OHOS_STD/js_api_module)

[base/compileruntime/js_api_module/](base/compileruntime/js_api_module/readme.md)
