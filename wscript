# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('wireless-point-to-point', ['network', 'mpi'])
    module.source = [
        'model/wireless-point-to-point-net-device.cc',
        'model/wireless-point-to-point-channel.cc',
        'model/wppp-header.cc',
        'helper/wireless-point-to-point-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('wireless-point-to-point')
    module_test.source = [
        'test/wireless-point-to-point-test.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'wireless-point-to-point'
    headers.source = [
        'model/wireless-point-to-point-net-device.h',
        'model/wireless-point-to-point-channel.h',
        'model/wppp-header.h',
        'helper/wireless-point-to-point-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

