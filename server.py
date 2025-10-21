import socket

HOST = '127.0.0.1'      # Server socket ADDRESS
PORT = 5500             # Server socket PORT

# Creazione del socket TCP
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))    # ASSEGNAZIONE PORTA (BINDING)
    s.listen()              # IL SERVER RIMANE IN ASCOLTO
    print(f"Server in ascolto su {HOST}:{PORT}...")
    
    conn, addr = s.accept()                 # IL SERVER ACCETTA CONNESSIONE IN ARRIVO DA CLIENT (BLOCCANTE)
    with conn:
        print(f"Connessione stabilita con {addr}")
        while True:
            data = conn.recv(1024)              # RICEVO UN MASSIMO DI 1024 byte
            
            if not data:
                break
            
            print(f"Ricevuto: {data.decode()}") # STAMPO LA STRINGA RICEVUTA
            conn.sendall(data)                  # RIMANDO INDIETRTO I DATI RICEVUTI
