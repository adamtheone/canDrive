import subprocess

subprocess.check_output(['pyuic5', '-o', 'canSniffer_ui.py', 'canSniffer.ui'])
