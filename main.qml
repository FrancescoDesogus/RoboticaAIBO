import QtQuick 2.0

Rectangle {
    id: container

    // Costante per mappare i valori di distanza ricevuti dal robot ai pixel della canvas. Ho fatto che 100cm = 250 pixel
    property real distanceToPixels: 250 / 100

    // Costante ausialiaria per convertire da angoli in radianti a gradi
    property real radToDeg: Math.PI / 180.0;

    // Array che contiene le vittime mostrate nella mappa
    property var victims: [];

    // Raggio entro cui considerare una vittima ricevuta come nuova (se una vittima è entro 50 pixel da un'altra non viene considerata come una vittima nuova)
    property real victimsRadius: 50;

    // Grandezza dei pallini delle vittime nella mappa
    property real victimsPointSize: 9;

    // Orientazione corrente del robot
    property real currentOrientation: 0

    // Walking step costante, usato per sposare il punto del robot in avanti
    property real walkingStep: 2;

    // Grandezza, altezza e rotazione della finestra sono definite da costanti C++
    width: WINDOW_WIDTH
    height: WINDOW_HEIGHT


    // Signal connessi ad altri signal C++
    signal pointFound(real distance, real angle)
    signal victimFound(real distance)
    signal robotTurn(real angle)
    signal generateStep()



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
            color: "green"
            radius: 5
            rotation: 0

            // Linea che mostra l'orientazione del robot
            Rectangle {
                id: orientationLine
                width: 2
                height: 4
                color: robot.color
                x: parent.width / 2 - width / 2
                y: - parent.height + 3

                antialiasing: true
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

    }

    // Bottone per ruotare verso sinistra la mappa
    Rectangle {
        id: rotateLeft
        width: textLeft.width + 10;
        height: 20;

        anchors.top: parent.top
        anchors.topMargin: 5;
        anchors.left: parent.left
        anchors.leftMargin: 5

        color: "red";
        radius: 5;
        border.color: "black"
        border.width: 1

        Text {
            id: textLeft
            text: qsTr("Rotate left");
            anchors.horizontalCenter: parent.horizontalCenter;
            anchors.verticalCenter: parent.verticalCenter;
            color: "white";
        }


        MouseArea {
            anchors.fill: parent
            onClicked: mapCanvas.rotation -= 90 % 360;
        }
    }

    // Bottone per ruotare verso destra la mappa
    Rectangle {
        id: rotateRight
        width: textRight.width + 10;
        height: 20;

        anchors.top: parent.top
        anchors.topMargin: 5;
        anchors.left: rotateLeft.right
        anchors.leftMargin: 5

        color: "red";
        radius: 5;
        border.color: "black"
        border.width: 1

        Text {
            id: textRight
            text: qsTr("Rotate right");
            anchors.horizontalCenter: parent.horizontalCenter;
            anchors.verticalCenter: parent.verticalCenter;
            color: "white";
        }


        MouseArea {
            anchors.fill: parent
            onClicked: mapCanvas.rotation += 90 % 360;
        }
    }

    // Testo con il contatore delle vittime
    Text {
        id: victimsCounterText
        text: "Victims counter: " + victims.length
        anchors.top: rotateLeft.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        color: "black";
    }

    // Connetto i signal della parte C++ con quelli QML, in modo che se vengono emessi da C++ vengono chiamati gli appositi slot di QML
    Component.onCompleted: {
        window.pointFound.connect(container.pointFound);
        window.victimFound.connect(container.victimFound);
        window.robotTurn.connect(container.robotTurn);
        window.generateStep.connect(container.generateStep);
    }


    // Slot connesso all'omonimo signal C++ che viene chiamato quando si riceve un nuovo valore di distanza dal robot e l'angolo della testa durente la ricerca;
    // i parametri sono la distanza e l'angolo di rotazione della testa del robot
    onPointFound: {
        // Recupero la posizione in pixel della distanza trovata
        var pixelDistance = distance * distanceToPixels;

        // Aggiorno la rotazione del robot con l'angolo della testa
        robot.rotation = currentOrientation + angle;

        // Se il valore è troppo vicino o troppo lontano lo ignoro
        if(distance <= 5 || distance > 30)
            return;

        // Creo il puntino del muro
        var pointObstacle = Qt.createQmlObject('import QtQuick 2.0; Rectangle {color: "red"; width: 4; height: 4; radius: 5;}',
                                           mapCanvas,
                                           "pointObstacle");

        // Setto la posizione del puntino rispetto al robot e alla sua orientazione con un po' di trigonometria, e compenso per le
        // dimensioni del pallino in modo che sia centrato rispetto a dove il robot sta fissando
        pointObstacle.x = robot.x + Math.sin(robot.rotation * radToDeg) * pixelDistance + pointObstacle.width/2;
        pointObstacle.y = robot.y - Math.cos(robot.rotation * radToDeg) * pixelDistance + pointObstacle.height/2;
    }

    // Slot che viene eseguito quando è stata trovata una vittima; prende come parametro "distance", la distanza della vittima dal robot
    onVictimFound: {
        var pixelDistance = distance * distanceToPixels;

        // Calcolo le coordinate in cui sarà messa la vittima
        var x = robot.x + Math.sin(robot.rotation * radToDeg) * pixelDistance + victimsPointSize/2;
        var y = robot.y - Math.cos(robot.rotation * radToDeg) * pixelDistance + victimsPointSize/2;

        // Controllo se la vittima è troppo vicina ad un'altra già presente; nel caso ignoro questa vittima
        var victimsNear = false;

        // Scorro tutte le vittime per controllare se sono vicine a quella attuale
        for(var i = 0; i < victims.length && !victimsNear; i++)
        {
            var v = victims[i];

            // Calcolo la distanza euclidea tra le vittime
            var dis = Math.sqrt((x-v.x)*(x-v.x) + (y-v.y)*(y-v.y));

            // Se la distanza è minore del threshold, considero la vittima corrente troppo vicina ad un'altra
            if(dis < victimsRadius)
                victimsNear = true;
        }

        // Se la vittima non è troppo vicina ad un'altra proseguo
        if(!victimsNear)
        {
            // Creo il pallino della vittima
            var victim = Qt.createQmlObject('import QtQuick 2.0; Rectangle {color: "blue"; width: 9; height: 9; radius: 4;}',
                                            mapCanvas,
                                            "pointVictim");


            // Creo il cerchio che mostra il raggio entro cui nuove vittime non vengono riconosciute (in realtà è più piccolo della metà rispetto al raggio
            // realmente usato, sennò è brutto a vedersi)
            var circle = Qt.createQmlObject('import QtQuick 2.0; Rectangle {id: circle; opacity: 0.2; color:"blue"; width: ' + victimsRadius +'; height: ' + victimsRadius + '; radius: width*0.5;}',
                                            mapCanvas,
                                            "victimRadius");

            // Calcolo la posizione del cerchio del raggio
            circle.x = x - circle.width/2 + victim.width/2 ;
            circle.y = y - circle.height/2 + victim.height/2;

            // Setto la posizione del puntino rispetto al robot e alla sua orientazione con un po' di trigonometria, e compenso per le
            // dimensioni del pallino in modo che sia centrato rispetto a dove il robot sta fissando
            victim.x = x;
            victim.y = y;


            // Nell'array delle vittime inserisco solo le coordinate del cerchio, che sono le uniche cose che servono
            var pair = { x: victim.x, y: victim.y };

            // Inserisco le coordinate
            victims.push(pair);

            // Aggiorno il testo con il contatore delle vittime
            victimsCounterText.text = "Victims counter: " + victims.length;
        }
    }


    // Slot chiamato quando il robot sta' girando; prende come parametro il nuovo angolo del robot
    onRobotTurn: {
        currentOrientation = angle % 360;
        robot.rotation = currentOrientation;
    }


    // Slot chiamato quando il robot cammina in avnati; non prende nessun parametro, la camminata è fatta "ad occhio"
    onGenerateStep: {
        // Aggiorno l'orientamento del robot (se infatti aveva appena smesso di ruotarsi per la ricerca non punterebbe dritto)
        robot.rotation = currentOrientation;

        // Aggiorno la posizione del robot, spostandola di un valore "step" lungo la direzione che sta fissando
        robot.x = robot.x + Math.sin(robot.rotation * radToDeg) * walkingStep;
        robot.y = robot.y - Math.cos(robot.rotation * radToDeg) * walkingStep;

        // Recupero il contesto della canvas, in modo da tracciare il percorso
        var ctx = mapCanvas.getContext("2d");

        // Connetto il punto precedente con quello attuale (spostandola poco poco per far si che la linea sia centrata nel robot)
        ctx.lineTo(robot.x + robot.width/2, robot.y + robot.height/2);

        // Disegno la linea
        ctx.stroke();

        // Forzo la canvas a fare l'update
        mapCanvas.requestPaint();
    }
}
