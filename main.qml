import QtQuick 2.0

Rectangle {
    id: container

    // Costante per mappare i valori di distanza ricevuti dal robot ai pixel della canvas. Ho fatto che 100cm = 250 pixel
    property real distanceToPixels: 250 / 100

    // Costante ausialiaria per convertire da angoli in radianti a gradi
    property real radToDeg: Math.PI / 180.0;

    // Stato 0: il robot si è appena avviato o si è appena girato di 90 gradi, quindi bisogna inserire un punto ostacolo che è la parete più lontana
    // Stato 1: il robot è in fase di ricerca, quindi il robot è fermo e vengono aggiunti punti nelle direzioni che guarda mentre ruota
    // Stato 2: il robot sta camminando in avanti, quindi non si disegna un punto con il valore della distanza ricevuto ma si sposta solo il robot
    property int state: 0

    // Valore che contiene la posizione del muro più lontano rispetto al robot; usata per aggiornare la posizione del robot quando cammina in avanti
    property real farWallDistance: 0

    // Grandezza, altezza e rotazione della finestra sono definite da costanti C++
    width: WINDOW_WIDTH
    height: WINDOW_HEIGHT
    rotation: WINDOW_ROTATION


    // Signal connessi ad altri signal C++
    signal pointFound(real distance, real angle)
    signal robotStateChanged(int newState, real newAngle)
    signal terminateRobot()


    // Canvas che contiene tutta la parte visibile
    Canvas {
        id: mapCanvas
        anchors.fill: parent

        // Oggetto che corrisponde al robot
        Rectangle {
            id: robot

            width: 7
            height: 7
            x:  parent.width/2
            y: parent.height - 150
            color: "blue"
            radius: 5
            rotation: 0

            // Linea che mostra l'orientazione del robot
            Rectangle {
                id: orientationLine
                width: 2
                height: 4
                color: "blue"
                x: parent.width / 2 - width / 2
                y: - parent.height + 3

                antialiasing: true
            }
        }

        // tmp
        MouseArea {
            id: mouseA
            anchors.fill: parent

            property real angle: 0
            property real pos: 30

            onClicked: {
                container.state = 2;
    //            mouseA.angle -= 10
//                robot.rotation = mouseA.angle;
//                mouseA.pos -= 1
//                pointFound(28, mouseA.angle);

                terminateRobot();
            }
        }

        // Senza l'override dell'onPaint inizialmente lo schermo diventerebbe nero
        onPaint: {

        }

        // Quando la canvas diventa disponibile inizializzo il path che disegnerà il percorso del robot
        onAvailableChanged: {
            if(available)
            {
                var ctx = mapCanvas.getContext("2d")
                ctx.strokeStyle = '#3B3B3B)';
                ctx.beginPath();
                ctx.moveTo(robot.x + robot.width/2, robot.y + robot.height/2);
            }
        }


        // tmp
        property int time: 1;
        focus: true

        // Bisogna far combaciare i gradi della rotazione con il grado di rotazione del robot; se si fa questo
        // le misure ai lati dovrebbero uscire giuste in modo naturale
        Keys.onPressed: {
            if (event.key == Qt.Key_Right) {

                mouseA.angle += 2;
                mouseA.pos += 0.5 * time;

                pointFound(mouseA.pos, mouseA.angle);

                event.accepted = true;
            }
            else if (event.key == Qt.Key_Up) {
//                robot.rotation = mouseA.angle;
                mouseA.pos -= 1
                console.log("mouseA.pos: " + mouseA.pos)
                pointFound(mouseA.pos, mouseA.angle);

                event.accepted = true;
            }
            else if (event.key == Qt.Key_Down) {
                console.log("move left");
                event.accepted = true;
            }
            else if (event.key == Qt.Key_Shift) {
                console.log("2asdsa")
                container.robotStateChanged(1, -50);
                mouseA.angle = -50;
                mouseA.pos = 10;
//                robot.rotation = -50;
                time = 1;

                event.accepted = true;
            }
            else if (event.key == Qt.Key_Control) {
                container.robotStateChanged(0, 90);
                mouseA.angle = 0;
                mouseA.pos = 40;
//                robot.rotation += 90;

                pointFound(mouseA.pos, mouseA.angle);

                event.accepted = true;
            }
            else if (event.key == Qt.Key_1) {
                console.log("iaosad")
                container.robotStateChanged(2, 0);
//                mouseA.angle = -50;
//                mouseA.pos = 10;
//                robot.rotation = -50;
//                time = 1;

                event.accepted = true;
            }

        }
    }

    Component.onCompleted: {
        // Connetto i signal della parte C++ con quelli QML, in modo che se vengono emessi da C++ vengono chiamati gli appositi slot di QML
        window.pointFound.connect(container.pointFound);
        window.robotStateChanged.connect(container.robotStateChanged);

        container.terminateRobot.connect(window.terminateRobot);
    }

    property real currentOrientation: 0

    // Slot connesso all'omonimo signal C++ che viene chiamato quando si riceve un nuovo valore di distanza dal robot e l'angolo della testa;
    // i parametri sono la distanza e l'angolo di rotazione della testa del robot
    onPointFound: {
        // Recupero la posizione in pixel della distanza trovata
        var pixelDistance = distance * distanceToPixels;

        // Normalmente, l'angolo passato è 0 (nella fase di camminata). Diventa diverso da 0 quando inizia la fase di searching;
        // in tal caso l'angolo partirà sempre negativo e finirà positivo (in quanto la testa del robot si gira da sinista a destra);
        // di conseguenza per stabilire la rotazione del pallino che rappresenta il robot è sufficiente sommare l'angolo all'orientazione
        // attuale del robot
        robot.rotation = currentOrientation + angle;


//        console.log("Message received; state: " + container.state + "; distance (in centimeters): " + distance + "; angle: " + angle);


        // A seconda dello stato del robot vengono eseguite azioni diverse
        switch(container.state)
        {
        // Stato 0: il robot è appena partito o si è appena girato di 90 gradi, quindi quello che bisogna fare è salvarsi la posizione
        // della parete più lontana e disegnare un puntino rosso in quel punto (l'unica differenza con lo stato 1 è il fatto di dover
        // salvare la posizione della parete più lontana davanti a se, quindi ho accorpato i casi, motivo per cui manca il break)
        case 0:
            farWallDistance = pixelDistance;

        // Stato 1: il robot è in fase di ricerca, quindi devo solo disegnare i punti che trova; il robot sta fermo, gira solo la testa
        case 1:

            // Creo il puntino del muro
            var pointObstacle = Qt.createQmlObject('import QtQuick 2.0; Rectangle {color: "red"; width: 4; height: 4; radius: 5;}',
                                               mapCanvas,
                                               "pointObstacle");

            // Setto la posizione del puntino rispetto al robot e alla sua orientazione con un po' di trigonometria, e compenso per le
            // dimensioni del pallino in modo che sia centrato rispetto a dove il robot sta fissando
            pointObstacle.x = robot.x + Math.sin(robot.rotation * radToDeg) * pixelDistance + pointObstacle.width/2;
            pointObstacle.y = robot.y - Math.cos(robot.rotation * radToDeg) * pixelDistance + pointObstacle.height/2;

            break;
        case 2:
            // Calcolo lo step da eseguire
            var step = farWallDistance - pixelDistance;


            // Aggiorno la posizione del robot, spostandola di un valore "step" lungo la direzione che sta fissando
            robot.x = robot.x + Math.sin(robot.rotation * radToDeg) * step;
            robot.y = robot.y - Math.cos(robot.rotation * radToDeg) * step;


            // Aggiorno la distanza del muro rispetto alla posizione attuale del robot
            farWallDistance = pixelDistance;

            // Recupero il contesto della canvas, in modo da tracciare il percorso
            var ctx = mapCanvas.getContext("2d");

            // Connetto il punto precedente con quello attuale (spostandola poco poco per far si che la linea sia centrata nel robot)
            ctx.lineTo(robot.x + robot.width/2, robot.y + robot.height/2);

            // Disegno la linea
            ctx.stroke();

            // Forzo la canvas a fare l'update
            mapCanvas.requestPaint();

            break;
        }
    }

    // Slot connesso all'omonimo signal C++ che viene emesso quando cambia lo stato del robot
    onRobotStateChanged: {
        container.state = newState;

        switch(newState) // need angle nel caso in cui si giri di 90 gradi a sinistra o a destra
        {
        // Se siamo di nuovo nello stato 0, il robot ha girato 90 gradi, quindi aggiorno l'orientazione attuale con newAngle,
        // che può essere 90 o -90 a seconda dell'orientazione della rotazione
        case 0:
//            currentOrientation = (currentOrientation + newAngle) % 360;
            currentOrientation = newAngle % 360;
            break;
        // Se siamo nello stato 1, il robot inizia lo scanning partendo da un angolo noto a priori (negativo, gira la testa da sinistra)
        case 1:
//            robot.rotation += newAngle;
            break;
        // Se siamo in fase di camminata, qualora tornassimo in questa fase dopo la ricerca, riporto l'orientazione a quella attuale
        case 2:
            robot.rotation = currentOrientation;
            break;
        }
    }
}
