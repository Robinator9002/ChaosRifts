import os
import json
import glob
import sys

# --- KONFIGURATION ---
PROJECT_ROOT = os.getcwd()
RSP_DIR_NAME = ".vscode/compileCommands_ChaosRifts"
OUTPUT_FILE = os.path.join(PROJECT_ROOT, "compile_commands.json")
COMPILER_PATH = "/opt/unreal-engine/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v25_clang-18.1.0-rockylinux8/x86_64-unknown-linux-gnu/bin/clang++"

# --- SKRIPT-Logik (Version 4 - "Keine Geheimnisse mehr") ---

source_file_cache = {}

def find_source_file(search_paths, file_to_find):
    if file_to_find in source_file_cache:
        return source_file_cache[file_to_find]
    for search_path in search_paths:
        pattern = os.path.join(search_path, '**', file_to_find)
        results = glob.glob(pattern, recursive=True)
        if results:
            found_path = results[0]
            source_file_cache[file_to_find] = found_path
            return found_path
    return None

def main():
    print("Starte Konfiguration für VS Code IntelliSense (Version 4 - Final)...")
    rsp_directory = os.path.join(PROJECT_ROOT, RSP_DIR_NAME)
    if not os.path.isdir(rsp_directory):
        print(f"FEHLER: Der Ordner '{rsp_directory}' wurde nicht gefunden.", file=sys.stderr)
        sys.exit(1)

    all_commands = []
    search_paths = [os.path.join(PROJECT_ROOT, "Source"), os.path.join(PROJECT_ROOT, "Intermediate")]
    placeholder_file = os.path.join(PROJECT_ROOT, "ChaosRifts.uproject")

    rsp_files = glob.glob(os.path.join(rsp_directory, "*.rsp"))
    print(f"{len(rsp_files)} .rsp-Dateien gefunden. Verarbeite...")

    for rsp_path in rsp_files:
        base_name = os.path.basename(rsp_path)
        source_file_name = '.'.join(base_name.split('.')[:-2])
        full_source_path = find_source_file(search_paths, source_file_name)
        
        if not full_source_path:
            full_source_path = placeholder_file

        # --- DIE ENTSCHEIDENDE ÄNDERUNG ---
        # Wir lesen jetzt den Inhalt der .rsp-Datei und fügen ihn direkt ein.
        try:
            with open(rsp_path, 'r') as f_rsp:
                rsp_args = f_rsp.read().splitlines()
                # Entferne eventuelle Anführungszeichen, die der Shell Probleme machen könnten
                expanded_command_args = ' '.join(f'"{arg}"' if ' ' in arg else arg for arg in rsp_args)
        except IOError:
            print(f"WARNUNG: Konnte {rsp_path} nicht lesen.", file=sys.stderr)
            continue # Überspringe diese Datei

        command_entry = {
            "directory": PROJECT_ROOT,
            "command": f'{COMPILER_PATH} {expanded_command_args}', # Kein @-Symbol mehr!
            "file": full_source_path,
        }
        all_commands.append(command_entry)

    print(f"\nVerarbeitung abgeschlossen. {len(all_commands)} Einträge werden geschrieben.")
    
    with open(OUTPUT_FILE, 'w') as f:
        json.dump(all_commands, f, indent=4)
    
    print(f"Fertig! '{OUTPUT_FILE}' wurde mit vollständig expandierten Befehlen erstellt.")
    print("Bitte lade VS Code jetzt ein letztes Mal neu.")

if __name__ == "__main__":
    main()