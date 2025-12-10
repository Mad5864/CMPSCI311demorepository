import pygame
import sys
import threading
import socket
import os
import time

# --- Configuration ---
# Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
GRAY = (220, 220, 220)
BLUE = (0, 100, 255)
RED = (255, 50, 50)

# Screen dimensions
SCREEN_WIDTH = 600
SCREEN_HEIGHT = 450
FONT_SIZE = 20

# Networking details (must match your C server)
SERVER_HOST = "127.0.0.1"
SERVER_PORT = 5000
MESSAGE_SIZE = 80 # Max buffer size for receiving

# --- Global State ---
chat_history = [] # Stores messages for display
input_text = ""
active_input = True 
client_socket = None
username = ""
connection_status = "Connecting..."
running = True # Control flag for main loop and threads

# --- Pygame Initialization ---
pygame.init()
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption("TCP Chat Client")
font = pygame.font.SysFont('Arial', FONT_SIZE)

# --- UI Components Rects ---
DISPLAY_RECT = pygame.Rect(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 60)
INPUT_RECT = pygame.Rect(10, SCREEN_HEIGHT - 40, SCREEN_WIDTH - 20, 30)

# --- Networking Functions ---

def receive_messages():
    """Thread function to continuously listen for and process server messages."""
    global chat_history, connection_status, running

    while running:
        try:
            # Receive data (blocking call)
            data = client_socket.recv(MESSAGE_SIZE)
            
            if not data:
                # Server disconnected (len <= 0 in C code)
                connection_status = "Server Disconnected"
                chat_history.append(("System", "Server disconnected."))
                running = False
                break
            
            message = data.decode('utf-8').strip()
            
            # NOTE: Your C client sends formatted messages like "User: Message".
            # This logic adds the received message to the chat history.
            
            # We assume the received message is already formatted for display
            # Look for the colon to separate sender and content
            if ":" in message:
                sender, content = message.split(":", 1)
                chat_history.append((sender.strip(), content.strip()))
            else:
                chat_history.append(("Server", message))

        except ConnectionResetError:
            connection_status = "Connection Lost"
            chat_history.append(("System", "Connection lost unexpectedly."))
            running = False
            break
        except Exception:
            # Handle other socket closing events or errors
            break

def connect_to_server():
    """Initializes the socket connection."""
    global client_socket, connection_status
    
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((SERVER_HOST, SERVER_PORT))
        connection_status = f"Connected as {username}"
        chat_history.append(("System", f"Successfully connected to {SERVER_HOST}:{SERVER_PORT}"))
        
        # Start the read thread
        threading.Thread(target=receive_messages, daemon=True).start()
        return True
    
    except socket.error as e:
        connection_status = f"Connection Failed: {e}"
        print(connection_status)
        chat_history.append(("System", connection_status))
        return False

def send_message(msg):
    """Sends a formatted message to the server."""
    if client_socket and running:
        try:
            # Format message: "%s: %s" as in tcp_client.c
            formatted_message = f"{username}: {msg}"
            client_socket.sendall(formatted_message.encode('utf-8'))
        except socket.error as e:
            print(f"Failed to send message: {e}")
            chat_history.append(("System", "Failed to send message."))

# --- Drawing Functions ---

def draw_text(surface, text, color, x, y):
    """Helper function to render text."""
    text_surface = font.render(text, True, color)
    surface.blit(text_surface, (x, y))

def draw_chat_history(surface):
    """Draws all messages in the chat history."""
    pygame.draw.rect(surface, GRAY, DISPLAY_RECT, 0)
    
    y_offset = DISPLAY_RECT.top + 5
    
    # Draw status bar at the top
    status_color = BLUE if "Connected" in connection_status else RED
    draw_text(surface, connection_status, status_color, DISPLAY_RECT.left + 5, y_offset)
    y_offset += FONT_SIZE + 5
    
    # Draw history
    # Start drawing from the bottom-most visible message up to keep history scrolling correct
    max_lines = (DISPLAY_RECT.height - (FONT_SIZE + 5)) // (FONT_SIZE + 2) # Calculate space for messages
    
    for sender, message in chat_history[max(-max_lines, -len(chat_history)):]:
        
        # Check if the message is from the system or a user
        if sender == "System":
            color = RED
            display_msg = f"[SYSTEM] {message}"
        else:
            color = BLACK
            display_msg = f"<{sender}>: {message}"
            
        draw_text(surface, display_msg, color, DISPLAY_RECT.left + 5, y_offset)
        y_offset += FONT_SIZE + 2

def draw_input_box(surface):
    """Draws the text input field."""
    # Draw the box border
    color = BLUE if active_input and running else BLACK
    pygame.draw.rect(surface, color, INPUT_RECT, 2)
    
    # Draw the current text
    draw_text(surface, input_text, BLACK, INPUT_RECT.left + 5, INPUT_RECT.top + 5)
    
    # Draw a blinking cursor
    if active_input and running and pygame.time.get_ticks() % 1000 < 500:
        cursor_x = INPUT_RECT.left + 5 + font.size(input_text)[0]
        # Prevent drawing cursor outside the box
        if cursor_x < INPUT_RECT.right - 5:
             pygame.draw.line(surface, BLACK, (cursor_x, INPUT_RECT.top + 5), 
                              (cursor_x, INPUT_RECT.bottom - 5), 2)

# --- Main Logic ---

def get_username_and_connect():
    """Initial step to get username before starting the Pygame window."""
    global username
    temp_name = input("Enter your username: ").strip()
    if not temp_name:
        username = f"PyUser{time.time():.0f}"
    else:
        username = temp_name
        
    return connect_to_server()

def run_client():
    global input_text, active_input, running, client_socket

    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break
            
            if not running:
                continue

            # --- Input Handling ---
            if event.type == pygame.MOUSEBUTTONDOWN:
                if INPUT_RECT.collidepoint(event.pos):
                    active_input = True
                else:
                    active_input = False

            if event.type == pygame.KEYDOWN and active_input:
                if event.key == pygame.K_RETURN:
                    # Send the message
                    if input_text:
                        # Add to local history instantly
                        chat_history.append((username, input_text)) 
                        send_message(input_text)
                        
                        input_text = "" # Clear input
                elif event.key == pygame.K_BACKSPACE:
                    input_text = input_text[:-1]
                else:
                    # Limit input length
                    if font.size(input_text + event.unicode)[0] < INPUT_RECT.width - 20:
                        input_text += event.unicode
        
        # --- Drawing ---
        screen.fill(WHITE)
        draw_chat_history(screen)
        draw_input_box(screen)

        pygame.display.flip()

    # --- Cleanup ---
    if client_socket:
        client_socket.close()
    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    # 1. Connect and get username first
    if get_username_and_connect():
        # 2. Run the main Pygame loop only if connection succeeds
        run_client()
    else:
        print("Failed to start chat client due to connection error.")
        sys.exit(1)