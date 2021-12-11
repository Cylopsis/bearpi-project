/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const helpUtil = requireInternal('util');
let TextEncoder = helpUtil.TextEncoder;
let TextDecoder = helpUtil.TextDecoder;
let RationalNumber = helpUtil.RationalNumber;
let Base64 = helpUtil.Base64;

function switchLittleObject(enter, obj, count)
{
    var str = '';
    if (obj === null) {
        str += obj;
    } else if (obj instanceof Array) {
        str += '[ ' + arrayToString(enter, obj, count) + '[length]: ' + obj.length + ' ]';
    } else if (typeof obj === 'function') {
        str += '{ [Function: ' + obj.name + ']' + enter
            + '[length]: ' + obj.length + ',' + enter
            + '[name] :\'' + obj.name + '\',' + enter
            + '[prototype]: ' + obj.name + ' { [constructor]: [Circular] } }';
    } else if (typeof obj === 'object') {
        str += '{ ';
        var i = 0;
        for (i in obj) {
            str += switchLittleValue(enter, i, obj, count);
        }
        if (i === 0) {
            return obj;
        }
        str = str.substr(0, str.length - enter.length - 1);
        str += ' }';
    } else if (typeof obj === 'string') {
        str += '\'' + obj + '\'';
    } else {
        str += obj;
    }
    return str;
}

function switchLittleValue(enter, protoName, obj, count)
{
    var str = '';
    if (obj[protoName] === null) {
        str += protoName + ': null,' + enter;
    } else if (obj[protoName] instanceof Array) {
        str += protoName + ':' + enter
            + '[ ' + arrayToString(enter + '  ', obj[protoName], count) + '[length]: '
            + obj[protoName].length + ' ],' + enter;
    } else if (typeof obj[protoName] === 'object') {
        if (obj[protoName] === obj) {
            str += protoName + ': [Circular]' + enter;
        } else {
            str += protoName + ':' + enter;
            str += switchLittleObject(enter + '  ', obj[protoName], count + 1) + ',' + enter;
        }
    } else if (typeof obj[protoName] === 'function') {
        var space = enter;
        if (obj[protoName].name !== '') {
            str += obj[protoName].name + ':' + space;
        }
        space += '  ';
        str += '{ [Function: ' + obj[protoName].name + ']' + space
            + '[length]: ' + obj[protoName].length + ',' + space
            + '[name] :\'' + obj[protoName].name + '\',' + space
            + '[prototype]: ' + obj[protoName].name
            + ' { [constructor]: [Circular] } },' + enter;
    } else {
        if (typeof obj[protoName] === 'string') {
            str += protoName + ': \'' + obj[protoName] + '\',' + enter;
        } else {
            str += protoName + ': ' + obj[protoName] + ',' + enter;
        }
    }
    return str;
}

function arrayToString(enter, arr, count)
{
    var str = '';
    if (!arr.length) {
        return '';
    }
    var i = 0;
    var arrayEnter = ', ';
    for (i in arr) {
        if (arr[i] !== null && (typeof arr[i] === 'function' || typeof arr[i] === 'object') && count <= 2) {
            arrayEnter += enter;
            break;
        }
    }
    i = 0;
    for (i in arr) {
        if (typeof arr[i] === 'string') {
            str += '\'' + arr[i].toString() + '\'' + arrayEnter;
        } else if (typeof arr[i] === 'object') {
            str += switchLittleObject(enter + '  ', arr[i], count + 1);
            str += arrayEnter;
        } else if (typeof arr[i] === 'function') {
            var space = enter;
            space += '  ';
            var end = '';
            if (arr[i].name !== '') {
                str += '{ [Function: ' + arr[i].name + ']' + space;
                end = arr[i].name + ' { [constructor]: [Circular] } }' + arrayEnter;
            } else {
                str += '{ [Function]' + space;
                end = '{ [constructor]: [Circular] } }' + arrayEnter;
            }
            str += '[length]: '
                + arr[i].length + ',' + space
                + '[name] :\'' + arr[i].name + '\',' + space
                + '[prototype]: ' + end;
        } else {
            str += arr[i] + arrayEnter;
        }
    }
    return str;
}

function switchBigObject(enter, obj, count)
{
    var str = '';
    if (obj === null) {
        str += obj;
    } else if (obj instanceof Array) {
        str += '[ ' + arrayToBigString(enter, obj, count) + ' ]';
    } else if (typeof obj === 'function') {
        str += '{ [Function: ' + obj.name + '] }';
    } else if (typeof obj === 'object') {
        str += '{ ';
        var i = 0;
        for (i in obj) {
            str += switchBigValue(enter, i, obj, count);
        }
        if (i === 0) {
            return obj;
        }
        str = str.substr(0, str.length - enter.length - 1);
        str += ' }';
    } else if (typeof obj === 'string') {
        str += '\'' + obj + '\'';
    } else {
        str += obj;
    }
    return str;
}

function switchBigValue(enter, protoName, obj, count)
{
    var str = '';
    if (obj[protoName] === null) {
        str += protoName + ': null,' + enter;
    } else if (obj[protoName] instanceof Array) {
        str += protoName + ':' + enter
            + '[ ' + arrayToBigString(enter + '  ', obj[protoName], count) + ' ],' + enter;
    } else if (typeof obj[protoName] === 'object') {
        if (obj[protoName] === obj) {
            str += protoName + ': [Circular]' + enter;
        } else {
            str += protoName + ':' + enter;
            str += switchBigObject(enter + '  ', obj[protoName], count + 1) + ',' + enter;
        }
    } else if (typeof obj[protoName] === 'function') {
        if (obj[protoName].name !== '') {
            str += obj[protoName].name + ': ';
        }
        str += '[Function: ' + obj[protoName].name + '],' + enter;
    } else {
        if (typeof obj[protoName] === 'string') {
            str += protoName + ': \'' + obj[protoName] + '\',' + enter;
        } else {
            str += protoName + ': ' + obj[protoName] + ',' + enter;
        }
    }
    return str;
}

function arrayToBigString(enter, arr, count)
{
    var str = '';
    if (!arr.length) {
        return '';
    }
    var i = 0;
    var arrayEnter = ', ';
    for (i in arr) {
        if (arr[i] !== null && (typeof arr[i] === 'object') && count <= 2) {
            arrayEnter += enter;
            break;
        }
    }
    i = 0;
    for (i in arr) {
        if (typeof arr[i] === 'string') {
            str += '\'' + arr[i] + '\'' + arrayEnter;
        } else if (typeof arr[i] === 'object') {
            str += switchBigObject(enter + '  ', arr[i], count + 1);
            str += arrayEnter;
        } else if (typeof arr[i] === 'function') {
            var end = '';
            if (arr[i].name !== '') {
                str += '[Function: ' + arr[i].name + ']' + arrayEnter;
            } else {
                str += '[Function]' + arrayEnter;
            }
        } else {
            str += arr[i] + arrayEnter;
        }
    }
    str = str.substr(0, str.length - arrayEnter.length);
    return str;
}

function switchIntValue(value)
{
    var str = '';
    if (value === '') {
        str += 'NaN';
    } else if (typeof value === 'bigint') {
        str += value + 'n';
    } else if (typeof value === 'symbol') {
        str += 'NaN';
    } else if (typeof value === 'number') {
        str += parseInt(value, 10); // 10:The function uses decimal.
    } else if (value instanceof Array) {
        if (typeof value[0] === 'number') {
            str += parseInt(value[0], 10); // 10:The function uses decimal.
        } else if (typeof value[0] === 'string') {
            if (isNaN(value[0])) {
                str += 'NaN';
            } else {
                str += parseInt(value[0], 10); // 10:The function uses decimal.
            }
        }
    } else if (typeof value === 'string') {
        if (isNaN(value)) {
            str += 'NaN';
        } else {
            str += parseInt(value, 10); // 10:The function uses decimal.
        }
    } else {
        str += 'NaN';
    }
    return str;
}

function switchFloatValue(value)
{
    var str = '';
    if (value === '') {
        str += 'NaN';
    } else if (typeof value === 'symbol') {
        str += 'NaN';
    } else if (typeof value === 'number') {
        str += value;
    } else if (value instanceof Array) {
        if (typeof value[0] === 'number') {
            str += parseFloat(value);
        } else if (typeof value[0] === 'string') {
            if (isNaN(value[0])) {
                str += 'NaN';
            } else {
                str += parseFloat(value[0]);
            }
        }
    } else if (typeof value === 'string') {
        if (isNaN(value)) {
            str += 'NaN';
        } else {
            str += parseFloat(value);
        }
    } else if (typeof value === 'bigint') {
        str += value;
    } else {
        str += 'NaN';
    }
    return str;
}

function switchNumberValue(value)
{
    var str = '';
    if (value === '') {
        str += '0';
    } else if (typeof value === 'symbol') {
        str += 'NaN';
    } else if (typeof value === 'number') {
        str += value;
    } else if (value instanceof Array) {
        str += 'NaN';
    } else if (typeof value === 'string') {
        if (isNaN(value)) {
            str += 'NaN';
        } else {
            str += Number(value);
        }
    } else if (typeof value === 'bigint') {
        str += value.toString() + 'n';
    } else {
        str += 'NaN';
    }
    return str;
}

function switchStringValue(value)
{
    var str = '';
    if (typeof value === 'undefined') {
        str += 'undefined';
    } else if (typeof value === 'object') {
        if (value === null) {
            str += 'null';
        } else {
            str += value;
        }
    } else if (typeof value === 'symbol') {
        str += value.toString();
    } else {
        str += value;
    }
    return str;
}

function printf(formatString, ...valueString)
{
    var formats = helpUtil.dealwithformatstring(formatString);
    var arr = [];
    arr = formats.split(' ');
    var switchString = [];
    var valueLength = valueString.length;
    var arrLength = arr.length;
    var i = 0;
    for (; i < valueLength && i < arrLength; i++) {
        if (arr[i] === 'o') {
            switchString.push(switchLittleObject('\n  ', valueString[i], 1));
        } else if (arr[i] === 'O') {
            switchString.push(switchBigObject('\n  ', valueString[i], 1));
        } else if (arr[i] === 'i') {
            switchString.push(switchIntValue(valueString[i]));
        } else if (arr[i] === 'j') {
            switchString.push(JSON.stringify(valueString[i]));
        } else if (arr[i] === 'd') {
            switchString.push(switchNumberValue(valueString[i]));
        } else if (arr[i] === 's') {
            switchString.push(switchStringValue(valueString[i]));
        } else if (arr[i] === 'f') {
            switchString.push(switchFloatValue(valueString[i]));
        } else if (arr[i] === 'c') {
            switchString.push(valueString[i].toString());
        }
    }
    while (i < valueLength) {
        switchString.push(valueString[i].toString());
        i++;
    }
    var helpUtilString = helpUtil.printf(formatString, ...switchString);
    return helpUtilString;
}

function getErrorString(errnum)
{
    var errorString = helpUtil.geterrorstring(errnum);
    return errorString;

}

function callbackified(original, ...args)
{
    const maybeCb = args.pop();
    if (typeof maybeCb !== 'function') {
        throw new Error('maybe is not function');
    }
    const cb = (...args) => {
        Reflect.apply(maybeCb, this, args);
    };
    Reflect.apply(original, this, args).then((ret) => cb(null, ret), (rej) => cb(rej));
}

function callbackWrapper(original)
{
    if (typeof original !== 'function') {
        throw new Error('original is not function');
    }
    const descriptors = Object.getOwnPropertyDescriptors(original);
    if (typeof descriptors.length.value === 'number') {
        descriptors.length.value++;
    }
    if (typeof descriptors.name.value === 'string') {
        descriptors.name.value += 'callbackified';
    }

    function cb(...args) {
        callbackified(original, ...args);
    }

    Object.defineProperties(cb, descriptors);
    return cb;
}

function promiseWrapper(func)
{
    return function (...args) {
        return new Promise((resolve, reject) => {
            let callback = function (err, ...values) {
                if (err) {
                    reject(err);
                } else {
                    resolve(values);
                }
            };
            func.apply(null, [...args, callback]);
        });
    };
}

class LruBuffer {
    constructor(capacity)
    {
        this.maxSize = 64;
        this.putCount = 0;
        this.createCount = 0;
        this.evictionCount = 0;
        this.hitCount = 0;
        this.missCount = 0;
        if (capacity !== undefined) {
            if (capacity <= 0) {
                throw new Error('data error');
            }
            this.maxSize = capacity;
        }
        this.cache = new Map();
    }

    updateCapacity(newCapacity)
    {
        if (newCapacity <= 0) {
            throw new Error('data error');
        }
        else if (this.cache.size > newCapacity) {
            this.changeCapacity(newCapacity);
        }
        this.maxSize = newCapacity;
    }

    get(key)
    {
        if (key === null) {
            throw new Error('key not be null');
        }
        let value;
        if (this.cache.has(key)) {
            value = this.cache.get(key);
            this.hitCount++;
            this.cache.delete(key);
            this.cache.set(key, value);
            return value;
        }
        this.missCount++;
        let createValue = this.createDefault(key);
        if (createValue === undefined) {
            return undefined;
        }
        else {
            value = this.put(key, createValue);
            this.createCount++;
            if (value !== null) {
                this.put(key, value);
                this.afterRemoval(false, key, createValue, value);
                return value;
            }
            return createValue;
        }
    }

    put(key, value)
    {
        if (key === null || value === null) {
            throw new Error('key or value key or value not be null');
        }
        let former;
        this.putCount++;
        if (this.cache.has(key)) {
            former = this.cache.get(key);
            this.cache.delete(key);
            this.afterRemoval(false, key, former, null);
        }
        else if (this.cache.size >= this.maxSize) {
            this.cache.delete(this.cache.keys().next().value);
            this.evictionCount++;
        }
        this.cache.set(key, value);
        return former;
    }

    getCreatCount()
    {
        return this.createCount;
    }

    getMissCount()
    {
        return this.missCount;
    }

    getRemovalCount()
    {
        return this.evictionCount;
    }

    getMatchCount()
    {
        return this.hitCount;
    }

    getPutCount()
    {
        return this.putCount;
    }

    capacity()
    {
        return this.maxSize;
    }

    size()
    {
        return this.cache.size;
    }

    clear()
    {
        this.cache.clear();
        this.afterRemoval(false, this.cache.keys(), this.cache.values(), null);
    }

    isEmpty()
    {
        let temp = false;
        if (this.cache.size === 0) {
            temp = true;
        }
        return temp;
    }

    contains(key)
    {
        let flag = false;
        if (this.cache.has(key)) {
            flag = true;
            let value;
            this.hitCount++;
            value = this.cache.get(key);
            this.cache.delete(key);
            this.cache.set(key, value);
            return flag;
        }
        this.missCount++;
        return flag;
    }

    remove(key)
    {
        if (key === null) {
            throw new Error('key not be null');
        }
        else if (this.cache.has(key)) {
            let former;
            former = this.cache.get(key);
            this.cache.delete(key);
            if (former !== null) {
                this.afterRemoval(false, key, former, null);
                return former;
            }
        }
        return undefined;
    }

    toString()
    {
        let peek = 0;
        let hitRate = 0;
        peek = this.hitCount + this.missCount;
        if (peek !== 0) {
            hitRate = 100 * this.hitCount / peek;
        }
        else {
            hitRate = 0;
        }
        let str = '';
        str = 'Lrubuffer[ maxSize = ' + this.maxSize + ', hits = ' + this.hitCount + ', misses = ' + this.missCount
            + ', hitRate = ' + hitRate + '% ]';
        return str;
    }

    values()
    {
        let arr = [];
        for (let value of this.cache.values()) {
            arr.push(value);
        }
        return arr;
    }

    keys()
    {
        let arr = [];
        for (let key of this.cache.keys()) {
            arr.push(key);
        }
        return arr;
    }

    afterRemoval(isEvict, key, value, newValue)
    {

    }

    createDefault(key)
    {
        return undefined;
    }

    entries()
    {
        let arr = [];
        for (let entry of this.cache.entries()) {
            arr.push(entry);
        }
        return arr;
    }

    [Symbol.iterator]()
    {
        let arr = [];
        for (let [key, value] of this.cache) {
            arr.push([key, value]);
        }
        return arr;
    }

    changeCapacity(newCapacity)
    {
        while (this.cache.size > newCapacity) {
            this.cache.delete(this.cache.keys().next().value);
            this.evictionCount++;
            this.afterRemoval(true, this.cache.keys(), this.cache.values(), null);
        }
    }
}
class Scope {
    constructor(lowerObj, upperObj)
    {
        this.lowerObj = lowerObj;
        this.upperObj = upperObj;
        this.checkNull(lowerObj, 'lower limit not be null');
        this.checkNull(upperObj, 'upper limit not be null');
        if (lowerObj.compareTo(upperObj)) {
            throw new Error('lower limit must be less than or equal to upper limit');
        }
        this._lowerLimit = lowerObj;
        this._upperLimit = upperObj;
    }

    getLower()
    {
        return this._lowerLimit;
    }

    getUpper()
    {
        return this._upperLimit;
    }

    contains(x)
    {
        let resLower;
        let resUpper;
        this.checkNull(x, 'value must not be null');
        if (x instanceof Scope) {
            resLower = x._lowerLimit.compareTo(this._lowerLimit);
            resUpper = this._upperLimit.compareTo(x._upperLimit);
        } else {
            resLower = x.compareTo(this._lowerLimit);
            resUpper = this._upperLimit.compareTo(x);
        }
        return resLower && resUpper;
    }

    clamp(value)
    {
        this.checkNull(value, 'value must not be null');
        if (!value.compareTo(this._lowerLimit)) {
            return this._lowerLimit;
        } else if (value.compareTo(this._upperLimit)) {
            return this._upperLimit;
        } else {
            return value;
        }
    }

    intersect(x, y)
    {
        let reLower;
        let reUpper;
        let mLower;
        let mUpper;
        if (y) {
            this.checkNull(x, 'lower limit must not be null');
            this.checkNull(y, 'upper limit must not be null');
            reLower = this._lowerLimit.compareTo(x);
            reUpper = y.compareTo(this._upperLimit);
            if (reLower && reUpper) {
                return this;
            } else {
                mLower = reLower ? this._lowerLimit : x;
                mUpper = reUpper ? this._upperLimit : y;
                return new Scope(mLower, mUpper);
            }
        } else {
            this.checkNull(x, 'scope must not be null');
            reLower = this._lowerLimit.compareTo(x._lowerLimit);
            reUpper = x._upperLimit.compareTo(this._upperLimit);
            if (!reLower && !reUpper) {
                return x;
            } else if (reLower && reUpper) {
                return this;
            } else {
                mLower = reLower ? this._lowerLimit : x._lowerLimit;
                mUpper = reUpper ? this._upperLimit : x._upperLimit;
                return new Scope(mLower, mUpper);
            }
        }
    }

    expand(x, y)
    {
        let reLower;
        let reUpper;
        let mLower;
        let mUpper;
        if (!y) {
            this.checkNull(x, 'value must not be null');
            if (!(x instanceof Scope)) {
                this.checkNull(x, 'value must not be null');
                return this.expand(x, x);
            }
            let reLower = x._lowerLimit.compareTo(this._lowerLimit);
            let reUpper = this._upperLimit.compareTo(x._upperLimit);
            if (reLower && reUpper) {
                return this;
            } else if (!reLower && !reUpper) {
                return x;
            } else {
                let mLower = reLower ? this._lowerLimit : x._lowerLimit;
                let mUpper = reUpper ? this._upperLimit : x._upperLimit;
                return new Scope(mLower, mUpper);
            }
        }
        else {
            this.checkNull(x, 'lower limit must not be null');
            this.checkNull(y, 'upper limit must not be null');
            let reLower = x.compareTo(this._lowerLimit);
            let reUpper = this._upperLimit.compareTo(y);
            if (reLower && reUpper) {
                return this;
            }
            let mLower = reLower ? this._lowerLimit : x;
            let mUpper = reUpper ? this._upperLimit : y;
            return new Scope(mLower, mUpper);
        }
    }

    toString()
    {
        let strLower = this._lowerLimit.toString();
        let strUpper = this._upperLimit.toString();
        return `[${strLower}, ${strUpper}]`;
    }

    checkNull(o, str)
    {
        if (o == null) {
            throw new Error(str);
        }
    }
}
export default {
    printf: printf,
    getErrorString: getErrorString,
    callbackWrapper: callbackWrapper,
    promiseWrapper: promiseWrapper,
    TextEncoder: TextEncoder,
    TextDecoder: TextDecoder,
    RationalNumber: RationalNumber,
    Base64: Base64,
    LruBuffer: LruBuffer,
    Scope: Scope,
};