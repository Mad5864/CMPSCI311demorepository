import pygame as pg
import socket
import threading

#--------------------- model -------------------------------------------------#

class Message:
    def __init__(self, name, message):
        self.name = name
        self.message = message
        self.next = None

class MessageList:
    def __init__(self):
        self.head = None
    
    def add(self, name, message):
        newMessage = Message(name, message)
        if self.head is None:
            self.head = newMessage
        else:
            newMessage.next = self.head
            self.head = newMessage

#--------------------- view  -------------------------------------------------#

class Label:
    def __init__(self, text, font):
        self.text = text
        self.font = font
    
    def draw(self, surface, x, y, color):
        surface.blit(self.font.render(self.text, True, color), (x - 8, y - 15))

class Rectangle:
    def __init__(self, topLeft, size):
        self.rect = (topLeft[0], topLeft[1], size[0], size[1])

    def hasMouse(self):
        (x,y) = pg.mouse.get_pos()
        left = self.rect[0]
        right = self.rect[0] + self.rect[2]
        up = self.rect[1]
        down = self.rect[1] + self.rect[3]
        return left < x < right and up < y < down
    
    def draw(self, surface, color):
        pg.draw.rect(surface, color, self.rect)

class Button:
    def __init__(self, panel, text, onColor, offColor):
        self.panel = panel
        self.text = text
        self.onColor = onColor
        self.offColor = offColor
    
    def hasMouse(self):
        return self.panel.hasMouse()

    def draw(self, surface):
        panelColor = self.offColor
        textColor = self.onColor
        if self.hasMouse():
            panelColor = self.onColor
            textColor = self.offColor
        self.panel.draw(surface, panelColor)
        self.text.draw(surface, self.panel.rect[0] + 15, self.panel.rect[1] + 15, textColor)

class InputField:
    def __init__(self, text, panel):
        self.text = text
        self.panel = panel
        self.ready = False
        self.active = False
    
    def hasMouse(self):
        return self.panel.hasMouse()
    
    def handleKeyPress(self, event):
        if event.key == pg.K_RETURN:
            self.ready = True
        if event.key == pg.K_BACKSPACE:
            self.text.text = self.text.text[:-1]
        else:
            key = pg.key.name(event.key)
            if key.isalnum() and len(key) == 1:
                self.text.text += key

    def draw(self, surface, panelColor, textColor):
        if self.active:
            panelColor, te
