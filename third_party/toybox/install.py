#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (c) 2020 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import argparse
import os
import sys


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--long_path', help='Toybox cmd long path', required=True)
    parser.add_argument(
        '--out_dir', help='Build out directoty', required=True)
    args = parser.parse_args()

    out_dir = args.out_dir
    bin_dir = os.path.join(out_dir, 'bin')
    if not os.path.exists(bin_dir):
        os.makedirs(bin_dir)

    sbin_dir = os.path.join(out_dir, 'sbin')
    if not os.path.exists(sbin_dir):
        os.makedirs(sbin_dir)
    
    usr_bin_dir = os.path.join(out_dir, 'usr', 'bin')
    if not os.path.exists(usr_bin_dir):
        os.makedirs(usr_bin_dir)

    usr_sbin_dir = os.path.join(out_dir, 'usr', 'sbin')
    if not os.path.exists(usr_sbin_dir):
        os.makedirs(usr_sbin_dir)

    # Making links by toybox long command.
    target_link = args.long_path
    if os.path.exists(target_link):
        os.remove(target_link)

    if target_link.find("usr") != -1:
        os.symlink("../../bin/toybox", target_link)
    if target_link.find("sbin") != -1:
        os.symlink("../bin/toybox", target_link)
    else:
        os.symlink("toybox", target_link)

    return 0


if __name__ == '__main__':
    sys.exit(main())

