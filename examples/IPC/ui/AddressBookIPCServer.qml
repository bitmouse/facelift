import QtQuick 2.0
import qface.addressbook 1.0

import QtQuick.Controls 1.2

import "../../AddressBook/ui"

Item {

    id: root
    height: mainPanel.height
    width: mainPanel.width

    Rectangle {
        anchors.fill: parent
	    color: "green"
    }

    AddressBookMainPanel {
        id: mainPanel
    	anchors.fill: parent
    	viewModel: viewModel
    }

    AddressBook {
        id: viewModel
    }

    // This object is used to register our model object on the bus
    AddressBookIPCAdapter {
//        objectPath: "/abook/viewmodel"
        service: viewModel
    }

}