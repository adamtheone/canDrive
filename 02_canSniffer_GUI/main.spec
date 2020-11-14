# -*- mode: python ; coding: utf-8 -*-
import os
from PyInstaller.utils.hooks import collect_data_files

spec_root = os.path.abspath(SPECPATH) 
qtmodern_data = collect_data_files('qtmodern')

block_cipher = None

a = Analysis(['main.py'],
             pathex=[spec_root],
             binaries=[],
             datas= qtmodern_data,
             hiddenimports=['PyQt5', 'Pyserial', 'PyQt5-tools', 'qtmodern'],
             hookspath=[],
             runtime_hooks=[],
             excludes=[],
             win_no_prefer_redirects=False,
             win_private_assemblies=False,
             cipher=block_cipher,
             noarchive=False)
pyz = PYZ(a.pure, a.zipped_data,
             cipher=block_cipher)
exe = EXE(pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          [],
          name='main',
          debug=False,
          bootloader_ignore_signals=False,
          strip=False,
          upx=True,
          upx_exclude=[],
          runtime_tmpdir=None,
          console=False )
