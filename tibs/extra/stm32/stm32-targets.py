#!/bin/env python3
#
# This script will build a couple of comma-separated-values (CSV) file
# from XML files with STM32 eval boards and MCUs. These files can be found
# in STM32CubeMX directory, and they are named stm32targets*.xml.
#
# Just run this script with a list of XML files on the command line and
# if parsing goes well you'll get stm32-board.csv and stm32-mcu.csv.
#
# Example:
#       ./stm32-targets.py --xmcu=STM32F7 --xmcu=STM32MP --xmcu=STM32L \
#               --xmcu=STM32U --xmcu=STM32W --xmcu=STM32G \
#               $(find /opt/STM32CubeMX -name 'stm32targets*.xml')
#

import sys, os, re
from xml.etree import ElementTree

namespace = "{http://st.com/stm32TargetDefinitions}"

# Unfortunately, STM32 database doesn't always contain the right macros to select
# the correct MCU in cmsis/stm32/*.h, so we'll figure it out ourselves given the
# rules in cmsis/stm32/stmf[01234]xx.h
device_defs = \
{
    "STM32F030x6": "STM32F030x4, STM32F030x6",
    "STM32F030x8": "STM32F030x8",
    "STM32F031x6": "STM32F031x4, STM32F031x6",
    "STM32F038xx": "STM32F038xx",
    "STM32F042x6": "STM32F042x4, STM32F042x6",
    "STM32F048x6": "STM32F048xx",
    "STM32F051x8": "STM32F051x4, STM32F051x6, STM32F051x8",
    "STM32F058xx": "STM32F058xx",
    "STM32F070x6": "STM32F070x6",
    "STM32F070xB": "STM32F070xB",
    "STM32F071xB": "STM32F071x8, STM32F071xB",
    "STM32F072xB": "STM32F072x8, STM32F072xB",
    "STM32F078xx": "STM32F078xx",
    "STM32F030xC": "STM32F030xC",
    "STM32F091xC": "STM32F091xB, STM32F091xC",
    "STM32F098xx": "STM32F098xx",
    "STM32F100xB": "STM32F100C4, STM32F100R4, STM32F100C6, STM32F100R6, STM32F100C8, STM32F100R8, STM32F100V8, STM32F100CB, STM32F100RB and STM32F100VB",
    "STM32F100xE": "STM32F100RC, STM32F100VC, STM32F100ZC, STM32F100RD, STM32F100VD, STM32F100ZD, STM32F100RE, STM32F100VE and STM32F100ZE",
    "STM32F101x6": "STM32F101C4, STM32F101R4, STM32F101T4, STM32F101C6, STM32F101R6 and STM32F101T6 Devices",
    "STM32F101xB": "STM32F101C8, STM32F101R8, STM32F101T8, STM32F101V8, STM32F101CB, STM32F101RB, STM32F101TB and STM32F101VB",
    "STM32F101xE": "STM32F101RC, STM32F101VC, STM32F101ZC, STM32F101RD, STM32F101VD, STM32F101ZD, STM32F101RE, STM32F101VE and STM32F101ZE",
    "STM32F101xG": "STM32F101RF, STM32F101VF, STM32F101ZF, STM32F101RG, STM32F101VG and STM32F101ZG",
    "STM32F102x6": "STM32F102C4, STM32F102R4, STM32F102C6 and STM32F102R6",
    "STM32F102xB": "STM32F102C8, STM32F102R8, STM32F102CB and STM32F102RB",
    "STM32F103x6": "STM32F103C4, STM32F103R4, STM32F103T4, STM32F103C6, STM32F103R6 and STM32F103T6",
    "STM32F103xB": "STM32F103C8, STM32F103R8, STM32F103T8, STM32F103V8, STM32F103CB, STM32F103RB, STM32F103TB and STM32F103VB",
    "STM32F103xE": "STM32F103RC, STM32F103VC, STM32F103ZC, STM32F103RD, STM32F103VD, STM32F103ZD, STM32F103RE, STM32F103VE and STM32F103ZE",
    "STM32F103xG": "STM32F103RF, STM32F103VF, STM32F103ZF, STM32F103RG, STM32F103VG and STM32F103ZG",
    "STM32F105xC": "STM32F105R8, STM32F105V8, STM32F105RB, STM32F105VB, STM32F105RC and STM32F105VC",
    "STM32F107xC": "STM32F107RB, STM32F107VB, STM32F107RC and STM32F107VC",
    "STM32F205xx": "STM32F205RG, STM32F205VG, STM32F205ZG, STM32F205RF, STM32F205VF, STM32F205ZF, STM32F205RE, STM32F205VE, STM32F205ZE, STM32F205RC, STM32F205VC, STM32F205ZC, STM32F205RB and STM32F205VB Devices",
    "STM32F215xx": "STM32F215RG, STM32F215VG, STM32F215ZG, STM32F215RE, STM32F215VE and STM32F215ZE Devices",
    "STM32F207xx": "STM32F207VG, STM32F207ZG, STM32F207IG, STM32F207VF, STM32F207ZF, STM32F207IF, STM32F207VE, STM32F207ZE, STM32F207IE, STM32F207VC, STM32F207ZC and STM32F207IC Devices",
    "STM32F217xx": "STM32F217VG, STM32F217ZG, STM32F217IG, STM32F217VE, STM32F217ZE and STM32F217IE Devices",
    "STM32F301x8": "STM32F301K6, STM32F301K8, STM32F301C6, STM32F301C8, STM32F301R6 and STM32F301R8 Devices",
    "STM32F302x8": "STM32F302K6, STM32F302K8, STM32F302C6, STM32F302C8, STM32F302R6 and STM32F302R8 Devices",
    "STM32F302xC": "STM32F302CB, STM32F302CC, STM32F302RB, STM32F302RC, STM32F302VB and STM32F302VC Devices",
    "STM32F302xE": "STM32F302RE, STM32F302VE, STM32F302ZE, STM32F302RD, STM32F302VD and STM32F302ZD Devices",
    "STM32F303x8": "STM32F303K6, STM32F303K8, STM32F303C6, STM32F303C8, STM32F303R6 and STM32F303R8 Devices",
    "STM32F303xC": "STM32F303CB, STM32F303CC, STM32F303RB, STM32F303RC, STM32F303VB and STM32F303VC Devices",
    "STM32F303xE": "STM32F303RE, STM32F303VE, STM32F303ZE, STM32F303RD, STM32F303VD and STM32F303ZD Devices",
    "STM32F373xC": "STM32F373C8, STM32F373CB, STM32F373CC, STM32F373R8, STM32F373RB, STM32F373RC, STM32F373V8, STM32F373VB and STM32F373VC Devices",
    "STM32F334x8": "STM32F334K4, STM32F334K6, STM32F334K8, STM32F334C4, STM32F334C6, STM32F334C8, STM32F334R4, STM32F334R6 and STM32F334R8 Devices",
    "STM32F318xx": "STM32F318K8, STM32F318C8: STM32F301x8 with regulator off: STM32F318xx Devices",
    "STM32F328xx": "STM32F328C8, STM32F328R8: STM32F334x8 with regulator off: STM32F328xx Devices",
    "STM32F358xx": "STM32F358CC, STM32F358RC, STM32F358VC: STM32F303xC with regulator off: STM32F358xx Devices",
    "STM32F378xx": "STM32F378CC, STM32F378RC, STM32F378VC: STM32F373xC with regulator off: STM32F378xx Devices",
    "STM32F398xx": "STM32F398VE: STM32F303xE with regulator off: STM32F398xx Devices",
    "STM32F405xx": "STM32F405RG, STM32F405VG and STM32F405ZG Devices",
    "STM32F415xx": "STM32F415RG, STM32F415VG and STM32F415ZG Devices",
    "STM32F407xx": "STM32F407VG, STM32F407VE, STM32F407ZG, STM32F407ZE, STM32F407IG  and STM32F407IE Devices",
    "STM32F417xx": "STM32F417VG, STM32F417VE, STM32F417ZG, STM32F417ZE, STM32F417IG and STM32F417IE Devices",
    "STM32F427xx": "STM32F427VG, STM32F427VI, STM32F427ZG, STM32F427ZI, STM32F427IG and STM32F427II Devices",
    "STM32F437xx": "STM32F437VG, STM32F437VI, STM32F437ZG, STM32F437ZI, STM32F437IG and STM32F437II Devices",
    "STM32F429xx": "STM32F429VG, STM32F429VI, STM32F429ZG, STM32F429ZI, STM32F429BG, STM32F429BI, STM32F429NG, STM32F439NI, STM32F429IG  and STM32F429II Devices",
    "STM32F439xx": "STM32F439VG, STM32F439VI, STM32F439ZG, STM32F439ZI, STM32F439BG, STM32F439BI, STM32F439NG, STM32F439NI, STM32F439IG and STM32F439II Devices",
    "STM32F401xC": "STM32F401CB, STM32F401CC, STM32F401RB, STM32F401RC, STM32F401VB and STM32F401VC Devices",
    "STM32F401xE": "STM32F401CD, STM32F401RD, STM32F401VD, STM32F401CE, STM32F401RE and STM32F401VE Devices",
    "STM32F410Tx": "STM32F410T8 and STM32F410TB Devices",
    "STM32F410Cx": "STM32F410C8 and STM32F410CB Devices",
    "STM32F410Rx": "STM32F410R8 and STM32F410RB Devices",
    "STM32F411xE": "STM32F411CC, STM32F411RC, STM32F411VC, STM32F411CE, STM32F411RE and STM32F411VE Devices",
    "STM32F446xx": "STM32F446MC, STM32F446ME, STM32F446RC, STM32F446RE, STM32F446VC, STM32F446VE, STM32F446ZC, STM32F446ZE Devices",
    "STM32F469xx": "STM32F469AI, STM32F469II, STM32F469BI, STM32F469NI, STM32F469AG, STM32F469IG, STM32F469BG, STM32F469NG, STM32F469AE, STM32F469IE, STM32F469BE and STM32F469NE Devices",
    "STM32F479xx": "STM32F479AI, STM32F479II, STM32F479BI, STM32F479NI, STM32F479AG, STM32F479IG, STM32F479BG, STM32F479NG Devices",
    "STM32F412Cx": "STM32F412CEU and STM32F412CGU Devices",
    "STM32F412Zx": "STM32F412ZET, STM32F412ZGT, STM32F412ZEJ and STM32F412ZGJ Devices",
    "STM32F412Vx": "STM32F412VET, STM32F412VGT, STM32F412VEH and STM32F412VGH Devices",
    "STM32F412Rx": "STM32F412RET, STM32F412RGT, STM32F412REY and STM32F412RGY Devices",
    "STM32F413xx": "STM32F413CH, STM32F413MH, STM32F413RH, STM32F413VH, STM32F413ZH, STM32F413CG, STM32F413MG, STM32F413RG, STM32F413VG and STM32F413ZG Devices",
    "STM32F423xx": "STM32F423CH, STM32F423RH, STM32F423VH and STM32F423ZH Devices",
    "STM32H742xx": "STM32H742VI, STM32H742ZI, STM32H742AI, STM32H742II, STM32H742BI, STM32H742XI Devices",
    "STM32H743xx": "STM32H743VI, STM32H743ZI, STM32H743AI, STM32H743II, STM32H743BI, STM32H743XI Devices",
    "STM32H753xx": "STM32H753VI, STM32H753ZI, STM32H753AI, STM32H753II, STM32H753BI, STM32H753XI Devices",
    "STM32H750xx": "STM32H750V, STM32H750I, STM32H750X Devices",
    "STM32H747xx": "STM32H747ZI, STM32H747AI, STM32H747II, STM32H747BI, STM32H747XI Devices",
    "STM32H757xx": "STM32H757ZI, STM32H757AI, STM32H757II, STM32H757BI, STM32H757XI Devices",
    "STM32H745xx": "STM32H745ZI, STM32H745II, STM32H745BI, STM32H745XI Devices",
    "STM32H755xx": "STM32H755ZI, STM32H755II, STM32H755BI, STM32H755XI Devices",
    "STM32H7B0xx": "STM32H7B0ABIxQ, STM32H7B0IBTx, STM32H7B0RBTx, STM32H7B0VBTx, STM32H7B0ZBTx, STM32H7B0IBKxQ",
    "STM32H7A3xx": "STM32H7A3IIK6, STM32H7A3IIT6, STM32H7A3NIH6, STM32H7A3RIT6, STM32H7A3VIH6, STM32H7A3VIT6, STM32H7A3ZIT6",
    "STM32H7A3xxQ": "STM32H7A3QIY6Q, STM32H7A3IIK6Q, STM32H7A3IIT6Q, STM32H7A3LIH6Q, STM32H7A3VIH6Q, STM32H7A3VIT6Q, STM32H7A3AII6Q, STM32H7A3ZIT6Q",
    "STM32H7B3xx": "STM32H7B3IIK6, STM32H7B3IIT6, STM32H7B3NIH6, STM32H7B3RIT6, STM32H7B3VIH6, STM32H7B3VIT6, STM32H7B3ZIT6",
    "STM32H7B3xxQ": "STM32H7B3QIY6Q, STM32H7B3IIK6Q, STM32H7B3IIT6Q, STM32H7B3LIH6Q, STM32H7B3VIH6Q, STM32H7B3VIT6Q, STM32H7B3AII6Q, STM32H7B3ZIT6Q",
}

# Some additional widespread boards
extra_boards = \
{
    # See doc/hardware/stm32f030chev.jpg
    "STM32F030ChEv": "stm32f030f4px",
    # See doc/hardware/stm32f103bluepill.jpg
    "STM32F103BluePill": "stm32f103c8tx",
}

def gettext (el):
    if el is None:
        return "None"
    return el.text


def parseDefines (el, defines=None):
    if defines is None:
        defines = set ()

    cdefs = el.find (namespace + "CDefines")
    if cdefs is None:
        return defines

    for cdef in cdefs.findall (namespace + "CDefine"):
        if cdef.text.startswith ("-D"):
            cdef.text = cdef.text [2:]
        defines.add (cdef.text)

    return defines


def serieDefines (family, defines=None):
    if defines is None:
        defines = set ()

    while family is not None:
        v = csv_serie.get (family)
        if v is None:
            break

        family = v [0]
        for _ in v [1]:
            defines.add (_)

    return defines


def joinDefines (defines):
    defs = [_ for _ in defines]
    defs.sort ()
    return ';'.join (defs)


def parseComments (el):
    comments = {}
    for c in el:
        if c.tag == ElementTree.Comment:
            keyval = c.text.split ('=', 3)
            if len (keyval) == 2:
                comments [keyval [0].strip ()] = keyval [1].strip ()
    return comments


def startswith (text, pfx):
    if pfx is None:
        return False

    for _ in pfx:
        if text.startswith (_):
            return True
    return False


def parseXml (fn, xmcu, xboard):
    try:
        target = ElementTree.TreeBuilder (insert_comments=True)
        et = ElementTree.parse (fn, ElementTree.XMLParser(target=target, encoding='utf-8'))
    except Exception as e:
        print ("%s: %s" % (fn, str (e)))
        return

    for el in et.findall (namespace + "board"):
        try:
            name = el.find (namespace + "name").text
            if startswith (name, xboard):
                continue
            mcu = gettext (el.find (namespace + "parent"))
            csv_board [name] = (mcu)
        except Exception as ex:
            print ("%s: Error handling tag %s: %s" % (fn, el.tag, str (ex)))

    for el in et.findall (namespace + "serie"):
        name = el.find (namespace + "name").text
        parent = gettext (el.find (namespace + "parent"))
        defines = parseDefines (el)
        csv_serie [name] = [parent, defines]

    for el in et.findall (namespace + "mcu"):
        try:
            name = el.find (namespace + "name").text
            if startswith (name, xmcu):
                continue
            family = el.find (namespace + "parent").text
            core = el.find (namespace + "cpus").find (namespace + "cpu").find (namespace + "cores").find (namespace + "core")
            coretype = core.find (namespace + "type").text
            if coretype.startswith ("arm "):
                coretype = coretype [4:]
            if coretype.endswith ('+'):
                coretype = coretype [:-1] + "plus"
            corefpu = gettext (core.find (namespace + "fpu"))
            if corefpu == "None":
                corefpu = ""
            package = gettext (el.find (namespace + "package"))
            defines = parseDefines (el)

            ram = ("None", "None", "None")
            rom = ("None", "None", "None")
            for mem in el.find (namespace + "memories").findall (namespace + "memory"):
                memtype = mem.find (namespace + "type").text
                address = mem.find (namespace + "address").text
                address = int (address, 0)
                size = mem.find (namespace + "size").text
                size = int (size, 0)
                if memtype == "RAM":
                    ram = ("0x%08x" % address, "0x%x" % size, "0x%08x" % (address + size))
                elif memtype == "ROM":
                    rom = ("0x%08x" % address, "0x%x" % size, "0x%08x" % (address + size))

            serieDefines (family, defines)

            # Unfortunately, STVPdevicename doesn't always match the right #define for cmsis/stm32/*
            #comments = parseComments (el)
            #if "STVPdevicename" in comments:
            #    defines.add (comments ["STVPdevicename"])

            # So we have to find the right macro ourselves
            mcu_def = None
            for k,v1,v2 in device_defs:
                if v1.match (name):
                    mcu_def = k
                    break

            if mcu_def is None:
                for k,v1,v2 in device_defs:
                    if v2.match (name):
                        mcu_def = k
                        break

            if mcu_def is None:
                print ("Cannot figure out compiler #define for MCU %s, " \
                       "please add a new entry to device_defs[] if it is needed" % name)
                continue

            defines.add (mcu_def)
            csv_mcu [name] = (family, coretype, corefpu, package, joinDefines (defines), *ram, *rom)
        except Exception as ex:
            print ("%s: Error handling element: %s" % (fn, str (ex)))
            ElementTree.dump (el)


def write_csv (fn, header, csv):
    if len (csv) == 0:
        print ("No objects to write to ", fn)
        if os.access (fn, os.F_OK):
            os.unlink (fn)
        return

    of = open (fn, "w")
    of.write (header + "\n")
    for k,v in csv.items ():
        if type (v) == str:
            line = k + "," + v
        else:
            line = k + "," + ",".join (str (x) for x in v)
        of.write (line + "\n")
    of.close ()


def prepare_device_defs ():
    global device_defs

    # Translate verbal descriptions to regular expression
    new_device_defs = []

    keys = [_ for _ in device_defs.keys ()]
    keys.sort ()

    for k in keys:
        v = device_defs [k]

        rex1 = v.replace ("and", ",")                   \
                .replace ("Devices", "")                \
                .replace ("with regulator off", "")     \
                .replace (",", "|")                     \
                .replace (":", "|")                     \
                .replace (" ", "")                      \
                .replace ("xx", ".*")                   \
                .replace ("x", ".")                     \
                .replace ("XX", ".*")                   \
                .replace ("X", ".")
        rex1 = "(" + rex1 + ")"

        rex2 = k.replace ("xx", ".*")                   \
                .replace ("x", ".")                     \
                .replace ("XX", ".*")                   \
                .replace ("X", ".")

        new_device_defs.append ((k, re.compile (rex1, re.IGNORECASE), re.compile (rex2, re.IGNORECASE)))

    device_defs = new_device_defs


### MAIN ###

import argparse

parser = argparse.ArgumentParser (description='Extract evaluation board and MCU information from STM32 XML files')
parser.add_argument ('xmlfiles', nargs='*')
parser.add_argument ('--xboard', action='append',
                     help='exclude evaluation boards starting with given prefixes')
parser.add_argument ('--xmcu', action='append',
                     help='exclude MCU starting with given prefixes')

prepare_device_defs ()

args = parser.parse_args()

if len (args.xmlfiles) == 0:
    parser.print_help ()
    sys.exit (-1)

csv_board = {}
csv_mcu = {}
csv_serie = {}

# Extra boards
for k,v in extra_boards.items ():
    csv_board [k] = (v)

for fn in args.xmlfiles:
    parseXml (fn, args.xmcu, args.xboard)

print ("Loaded %d evaluation boards and %d MCUs" % (len (csv_board), len (csv_mcu)))
write_csv ("stm32-board.csv", "name,mcu", csv_board)
write_csv ("stm32-mcu.csv", "name,series,core,fpu,package,defines,ram_address,ram_size,ram_end,rom_address,rom_size,rom_end", csv_mcu)
