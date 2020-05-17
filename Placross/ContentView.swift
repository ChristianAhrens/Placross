//
//  ContentView.swift
//  Placross
//
//  Created by Christian Ahrens on 12.05.20.
//  Copyright © 2020 Christian Ahrens. All rights reserved.
//

import SwiftUI
import AVFoundation

struct ContentView: View {
    var body: some View {
        Text("Hello, World!")
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}

//var  myAudioPlayer = AVAudioPlayer()
//override func viewDidLoad() {
//super.viewDidLoad()
//// Do any additional setup after loading the view, typically from a nib.
//let myFilePathString=NSBundle.mainBundle().pathForResource("Armand Zildjian Artist In Residence //Concert- Nate Smith Solo - 150BPM", ofType: "m4a")
//if let myFilePathString = myFilePathString
//{
//let myFilePathURL=NSURL(fileURLWithPath: myFilePathString)
//do{
//try  myAudioPlayer = AVAudioPlayer(contentsOfURL: myFilePathURL)
//myAudioPlayer.play()
//}
//catch{
//print("Error")
//}
//}
//}
