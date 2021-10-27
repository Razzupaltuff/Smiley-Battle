import PySimpleGUI as sg
import numpy as np

def Test1 ():
    # Ausgabe
    text = sg.Text("Guten Tag! Wie lautet Ihr Name?", 
                font=('Verdana', 18), 
                text_color='#ffffff', 
                background_color='#000000', 
                justification='center')

    # Eingabe
    eingabe = sg.Input(focus=True, 
                    enable_events=True, 
                    key='-EINGABE-')

    # Unsichtbar
    button = sg.Button('Enter', visible=False, bind_return_key=True)

    # Layout
    layout = [[text], [eingabe], [button]]

    # Fenster ohne Titelbar und Abstand zum Rand
    fenster = sg.Window('Meine erste GUI', layout, margins=(10, 10), resizable=True, grab_anywhere=True)

    # Endlos-Event-Schleife
    while True:
        event, werte = fenster.read()
        if event == sg.WINDOW_CLOSED:
            # Abbruchbedingung
            print("Tschüss")
            break
        elif event == '-EINGABE-' or not werte['-EINGABE-']:
            # Falls die Eingabe leer ist, kommt der Anfangssatz wieder
            text.Update("Guten Tag! Wie lautet Ihr Name?")
        elif event == 'Enter':
            # Der Benutzer wird mit seinem Namen begrüßt
            text.Update(f"Guten Tag {werte['-EINGABE-']}!")

    fenster.close()


def Popup():
    layout = [[sg.Button("Fehler"), sg.Button("Toast"), sg.Button("Ok")]]

    fenster = sg.Window("Popup Fenster", layout)

    while True:
        event, werte = fenster.read()

        if event == sg.WINDOW_CLOSED:
            break
        elif event == "Fehler":
            ergebnis = sg.PopupError("Es gab keinen Fehler, keine Sorge :)")    # Ergebnis immer "Error"
        elif event == "Toast":
            sg.PopupQuickMessage("Hier ist eine kurze Nachricht, die gleich wieder verschwindet.")
        elif event == "Ok":
            ergebnis = sg.PopupOKCancel("Alles hat geklappt!")  # Ergebnis entweder "Ok" oder "Cancel"
            print(ergebnis)

    fenster.close()


def HelperPopups ():
    layout = [[sg.Button("Text"), sg.Button("Datei"), sg.Button("Ordner"), sg.Button("Datum")]]

    fenster = sg.Window("Popup Fenster", layout)

    while True:
        event, werte = fenster.read()

        if event == sg.WINDOW_CLOSED:
            break
        elif event == "Text":
            # Popup, welcher eine Text zurückgibt, oder None, falls er abgebrochen wurde.
            ergebnis = sg.popup_get_text("Wie heißen Sie?")
        elif event == "Datei":
            # Popup, welcher einen Dateipfad zurückgibt, oder None, falls er abgebrochen wurde.
            ergebnis = sg.popup_get_file("Wo soll die Ausgabe gespeichert werden?")
        elif event == "Ordner":
            # Popup, welcher einen Orderpfad zurückgibt, oder None, falls er abgebrochen wurde.
            ergebnis = sg.popup_get_folder("Welcher Ordner soll geöffnet werden?")  
        elif event == "Datum":
            # Popup, welcher ein Datumstupel zurückgibt (jahr, monat, tag)
            ergebnis = sg.popup_get_date()
        # Popup mit dem Ergebnis, welches immer über allen anderen Fenstern steht
        sg.PopupQuickMessage("Diesen Wert hat das Popup zurückgegeben", ergebnis, keep_on_top=True)

    fenster.close()


def NestedLayout ():
# Funktion für die Cosinus-Berechnung
    def cos(x: np.array, amplitude: float=1, verschiebung: float=0) -> np.array:
        """
        :param x: Array von X Werten
        :param amplitude: Amplitude der Cosinus-Funktion
        :param verschiebung: Verschiebung der Cosinus-Funktion
        :return: Array von Y Werten
        """
        return np.cos(x*amplitude) + verschiebung

# Layout für die Seite mit zwei Schiebereglern für die Amplitude und Verschiebung und einen Button
    layout_einstellungen = [
        [sg.Text("Amplitude"), 
        sg.Text("Verschiebung")], 
        [sg.Slider(range=(-5, 5), resolution=0.1, default_value=1, key='-AMP-', enable_events=True), 
        sg.Slider(range=(-5, 5), resolution=0.1, default_value=0, key='-SCHIEBUNG-', enable_events=True)],
        [sg.Button('Putzen')]
    ]

    # Das normale Layout besitzt nur einen Text, den Canvas und das Unterfenster
    layout = [
        [sg.Text("Interaktive Cosinus-Funktion")],
        [sg.Canvas(key='-CANVAS-'), sg.Column(layout_einstellungen)]]

    fenster = sg.Window('Diagramm GUI', layout, finalize=True, margins=(10, 10))
    x = np.linspace(0, 5, 1000)
    while True:
        # Aktualisiert den Canvas
        event, werte = fenster.read()
        if event == sg.WINDOW_CLOSED:
            break
        else:
            # Liest die Werte von den Schiebereglern
            amplitude = werte['-AMP-']
            verschiebung = werte['-SCHIEBUNG-']

            # Berechnet die neuen Y-Werte
            y = cos(x, amplitude, verschiebung)

    fenster.close()

def ThemeTest ():
    layout = [[sg.Text('Wählen Sie ein Thema')],
            [sg.Listbox(values=sg.theme_list(), size=(30, 20), key='-THEMENLISTE-', enable_events=True)]]

    fenster = sg.Window('Themenauswahl', layout)

    while True: 
        event, werte = fenster.read()
        if event == sg.WINDOW_CLOSED:
            break
        print(werte)
        # Setzt das Thema der GUI mit dem ausgewählten Wert
        thema = werte['-THEMENLISTE-'][0]
        sg.theme(thema)
        # Beispiel des Themas
        sg.popup_get_text(f'Das aktuelle Thema ist nun {thema}')

    fenster.close()


def NoRetVal (x):
    y = x * x

z = NoRetVal (1)
print (z)

ThemeTest ()
