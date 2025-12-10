import pygame
import socket
import threading

WIDTH, HEIGHT = 600, 600
SERVER_HOST = "127.0.0.1"
SERVER_PORT = 5000   # MUST match C server port

pygame.init()
pygame.font.init()

FONT = pygame.font.SysFont("Arial", 20)
BG_COLOR = (30, 30, 30)
TEXT_COLOR = (240, 240, 240)
INPUT_BG = (50, 50, 50)
MESSAGE_BG = (40, 40, 40)

# USERNAME SETUP
username = input("Enter your username: ")

# CREATE WINDOW
window = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption(f"Chat Client - {username}")

messages = []
scroll_offset = 0
input_text = ""

# SOCKET SETUP
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    client_socket.connect((SERVER_HOST, SERVER_PORT))
    print("Connected to server.")
except Exception as e:
    print("Error connecting:", e)
    exit()

# Send username to server
client_socket.send(f"__username__:{username}".encode("utf-8"))

# RECEIVING THREAD
def receive_messages():
    global messages
    while True:
        try:
            msg = client_socket.recv(1024).decode("utf-8")
            if msg:
                messages.append(msg)
            else:
                break
        except:
            break


threading.Thread(target=receive_messages, daemon=True).start()


# DRAWING FUNCTIONS
def draw_messages(surface, scroll):
    y = 10 + scroll
    for msg in messages:
        rendered = FONT.render(msg, True, TEXT_COLOR)
        pygame.draw.rect(surface, MESSAGE_BG, (10, y, WIDTH - 20, rendered.get_height() + 6))
        surface.blit(rendered, (15, y + 3))
        y += rendered.get_height() + 12


def draw_input_box(surface, text):
    pygame.draw.rect(surface, INPUT_BG, (0, HEIGHT - 40, WIDTH, 40))
    rendered = FONT.render(text, True, TEXT_COLOR)
    surface.blit(rendered, (10, HEIGHT - 30))


# MAIN LOOP
running = True
clock = pygame.time.Clock()

while running:
    window.fill(BG_COLOR)

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

        if event.type == pygame.KEYDOWN:

            if event.key == pygame.K_RETURN:
                if input_text.strip() != "":
                    send_msg = f"{username}: {input_text}"
                    client_socket.send(send_msg.encode("utf-8"))
                input_text = ""

            elif event.key == pygame.K_BACKSPACE:
                input_text = input_text[:-1]

            else:
                if len(event.unicode) == 1:
                    input_text += event.unicode

        if event.type == pygame.MOUSEWHEEL:
            scroll_offset += event.y * 20

    # Draw messages
    message_surface = pygame.Surface((WIDTH, HEIGHT - 40))
    message_surface.fill(BG_COLOR)
    draw_messages(message_surface, scroll_offset)
    window.blit(message_surface, (0, 0))

    draw_input_box(window, input_text)
    pygame.display.update()
    clock.tick(60)

pygame.quit()
client_socket.close()
