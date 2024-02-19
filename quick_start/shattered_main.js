class Main {
    constructor() {
        this._viewer;
        this._viewerMode;
        this._reverseProxy;
        this._isLoading = false;
    }

    start (viewerMode, modelName, reverseProxy) {
        this._viewerMode = viewerMode.toUpperCase();;
        this._reverseProxy = reverseProxy;
        this.createViewer(this._viewerMode, modelName, this._reverseProxy);
        this.initEvents();
    }

    createViewer(viewerMode, modelName, reverseProxy) {
        createViewer(viewerMode, modelName, "container", reverseProxy).then((hwv) => {
            this._viewer = hwv;

            this._viewer.setCallbacks({
                sceneReady: () => {
                    this._viewer.view.setBackgroundColor(new Communicator.Color(19, 191, 243), new Communicator.Color(0, 87, 145));
                    this._viewer.view.setBackfacesVisible(true);
                    this._viewer.view.getAxisTriad().enable();
                    this._viewer.view.getNavCube().enable();
                },
                selectionArray: (selEvents) => {
                    const selCnt = selEvents.length;
                    if (0 == selCnt) return;

                    const selEvent = selEvents[selCnt - 1];
                    const selItem = selEvent.getSelection();
                    const node = selItem.getNodeId();
                    const name = this._viewer.model.getNodeName(node);
                    const mat = this._viewer.model.getNodeMatrix(node);
                    const netMat = this._viewer.model.getNodeNetMatrix(node);
                    console.log(node + ", " + name + ", matrix: " + mat.m[0].toFixed(0) + "," + mat.m[1].toFixed(0) + "," + mat.m[2].toFixed(0) + "," + mat.m[3].toFixed(0) + "," + mat.m[4].toFixed(0) + "," + mat.m[5].toFixed(0) + "," + mat.m[6].toFixed(0) + "," + mat.m[7].toFixed(0) + "," + mat.m[8].toFixed(0) + "," + mat.m[9].toFixed(0) + "," + mat.m[10].toFixed(0) + "," + mat.m[11].toFixed(0) + "," + mat.m[12].toFixed(0) + "," + mat.m[13].toFixed(0) + "," + mat.m[14].toFixed(0) + "," + mat.m[15].toFixed(0));
                    console.log("    Net matrix: " + netMat.m[0].toFixed(0) + "," + netMat.m[1].toFixed(0) + "," + netMat.m[2].toFixed(0) + "," + netMat.m[3].toFixed(0) + "," + netMat.m[4].toFixed(0) + "," + netMat.m[5].toFixed(0) + "," + netMat.m[6].toFixed(0) + "," + netMat.m[7].toFixed(0) + "," + netMat.m[8].toFixed(0) + "," + netMat.m[9].toFixed(0) + "," + netMat.m[10].toFixed(0) + "," + netMat.m[11].toFixed(0) + "," + netMat.m[12].toFixed(0) + "," + netMat.m[13].toFixed(0) + "," + netMat.m[14].toFixed(0) + "," + netMat.m[15].toFixed(0));
                }
            });

            this._viewer.start();
        });
    }

    initEvents() {
        let resizeTimer;
        let interval = Math.floor(1000 / 60 * 10);
        $(window).resize(() => {
        if (resizeTimer !== false) {
            clearTimeout(resizeTimer);
        }
        resizeTimer = setTimeout(() => {
            this._viewer.resizeCanvas();
        }, interval);
        });

        $("#loadXml").on("click", (e) => {
            if (this._isLoading)
                return alert("Now on loading.");
            
            this._isLoading = true;
            this._viewer.model.clear().then(() => {
                let startTime = Date.now();
                let rootNode = this._viewer.model.getAbsoluteRootNode();
                const xmlName = "shattered.xml";
                if ("CSR" == this._viewerMode || "SSR" == this._viewerMode) {
                    this._viewer.model.loadSubtreeFromXmlFile(rootNode, xmlName).then(() => {
                        alert(`Loading Time: ${Date.now() - startTime} milliseconds`);
                        this._isLoading = false;
                    });
                }
                else if ("SCS" == this._viewerMode) {
                    this._viewer.model.loadSubtreeFromScsXmlFile(rootNode, xmlName).then(() => {
                        alert(`Loading Time: ${Date.now() - startTime} milliseconds`);
                        this._isLoading = false;
                    });
                }
            });
        });
    }
}