import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'dart:convert'; // For JSON handling

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      home: HomePage(),
    );
  }
}

class HomePage extends StatefulWidget {
  @override
  _HomePageState createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  bool isAutomaticMode = false;
  int currentStage = 1;
  final PageController _pageController = PageController();
  late TabController _tabController;
  int _currentTabIndex = 0;

  // URL of the ESP32
  final String esp32Url =
      "http://192.168.21.43"; // Change this to your ESP32 IP

  // Function to send commands to the ESP32
  Future<void> sendCommand(String command) async {
    try {
      final response = await http.post(
        Uri.parse('$esp32Url/control'),
        headers: {'Content-Type': 'application/json'},
        body: json.encode({'command': command}),
      );

      if (response.statusCode == 200) {
        print("Command sent successfully");
      } else {
        print("Failed to send command: Stage $currentStage");
      }
    } catch (e) {
      print("Error: $e");
    }
  }

  // Toggle automatic mode on/off
  void toggleAutomaticMode() {
    setState(() {
      isAutomaticMode = !isAutomaticMode;
    });
    sendCommand(isAutomaticMode ? "automatic_on" : "automatic_off");
  }

  // Go to the next stage
  void goToNextStage() {
    setState(() {
      if (currentStage < 5) {
        currentStage++;
      } else {
        currentStage = 0;
      }
    });
    sendCommand("stage_$currentStage");
  }

  // Go to the previous stage
  void goToPreviousStage() {
    setState(() {
      if (currentStage > 0) {
        currentStage--;
      }
    });
    sendCommand("stage_$currentStage");
  }

  @override
  Widget build(BuildContext context) {
    return DefaultTabController(
      length: 2,
      child: Scaffold(
        backgroundColor: Colors.white,
        appBar: AppBar(
          centerTitle: true,
          title: const Text("Receptor-mediated endocytosis"),
          backgroundColor: Colors.black,
          foregroundColor: Colors.white,
          actions: [
            if (_currentTabIndex == 1)
              IconButton(
                icon: const Icon(
                  Icons.refresh,
                  color: Colors.white54,
                ),
                onPressed: () {
                  _pageController.jumpToPage(0);
                  setState(() {
                    currentStage = 0;
                    sendCommand("stage_$currentStage");
                  });
                },
              ),
          ],
          bottom: TabBar(
            labelColor: Colors.white,
            automaticIndicatorColorAdjustment: true,
            unselectedLabelColor: Colors.white60,
            dividerColor: Colors.white,
            indicatorColor: Colors.black,
            overlayColor: const WidgetStatePropertyAll(Colors.white30),
            onTap: (index) {
              setState(() {
                _currentTabIndex = index;
              });
            },
            tabs: const [
              Tab(text: 'Manual Mode'),
              Tab(text: 'Story Mode'),
            ],
          ),
        ),
        body: TabBarView(
          children: [
            // Manual Mode Tab
            buildManualMode(),
            // Story Mode Tab
            buildStoryMode(),
          ],
        ),
      ),
    );
  }

  // Build Manual Mode UI
  Widget buildManualMode() {
    return Column(
      children: [
        SizedBox(height: 20),
        GridView.count(
          crossAxisCount: 2,
          shrinkWrap: true,
          children: [
            buildControlButton("Automatic\nMode", toggleAutomaticMode),
            buildControlButton("Go to\nStage 0", toggleAutomaticMode),
            buildControlButton("Form\nMembrane", () => sendCommand("form")),
            buildControlButton(
                "Release\nMembrane", () => sendCommand("release")),
            buildControlButton("Previous\nStage", goToPreviousStage),
            buildControlButton("Next\nStage", goToNextStage),
          ],
        ),
      ],
    );
  }

  // Build individual control buttons
  Widget buildControlButton(String label, VoidCallback onPressed) {
    return Padding(
      padding: const EdgeInsets.all(16.0),
      child: ElevatedButton(
        style: ElevatedButton.styleFrom(
          backgroundColor: Colors.teal[50],
          foregroundColor: Colors.black,
          shape:
              RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
          padding: const EdgeInsets.symmetric(vertical: 20, horizontal: 20),
        ),
        onPressed: onPressed,
        child: Text(
          label,
          style: const TextStyle(fontSize: 18),
          textAlign: TextAlign.center,
        ),
      ),
    );
  }

  // Build Story Mode UI (with page swipe)
  Widget buildStoryMode() {
    return Center(
      child: PageView(
        controller: _pageController,
        onPageChanged: (index) {
          setState(() {
            if (index == 6) {
              _pageController.jumpToPage(0);
              currentStage = 0;
            } else {
              currentStage = index;
            }
            sendCommand("stage_$currentStage");
          });
        },
        children: [
          buildStorySlide("Stage 0\nInitial condition",
              "The plasma membrane is intact, with receptors embedded on its surface. Extracellular ligands or particles are present in the surrounding fluid, waiting to bind to specific receptors. The cell is in a homeostatic state, ready to initiate the process of receptor-mediated endocytosis in response to a stimulus."),
          buildStorySlide("Stage 1\nEndocytosis Initiation",
              "Receptor-ligand binding occurs at specialized regions called clathrin-coated pits or caveolae. The receptor on the cell surface binds to its specific ligand, triggering a signaling cascade that prepares the membrane for internalization. This is the cell’s response to external cues, marking the start of endocytosis."),
          buildStorySlide("Stage 2\nMembrane Invagination",
              "The plasma membrane begins to fold inward as the receptor-ligand complexes concentrate in the clathrin-coated pit or caveolae. Membrane curvature is facilitated by adaptor proteins like AP2 and clathrin, which help concentrate and bend the membrane, creating a shallow pocket that deepens as invagination continues."),
          buildStorySlide("Stage 3\nVesicle Formation",
              "As invagination progresses, the membrane becomes increasingly pinched, and the coated pit forms a vesicle. Dynamin, a GTPase, helps sever the vesicle from the plasma membrane. The vesicle is then fully formed, surrounded by a clathrin coat, which stabilizes the structure until it uncoats within the cytoplasm."),
          buildStorySlide("Stage 4\nVesicle Trafficking",
              "The newly formed vesicle detaches from the membrane and is transported along microtubules to early endosomes. Here, it may split into smaller vesicles, each destined for different intracellular locations. Sorting mechanisms direct specific cargo to either degradation or recycling pathways."),
          buildStorySlide("Stage 5\nMembrane Fusion",
              "Vesicles fuse with early endosomes, where the acidic environment facilitates dissociation of ligands from receptors. The receptor-ligand complex is sorted within the endosome, and the receptors are often recycled back to the cell surface. Fusion also facilitates further processing of internalized cargo, including degradation in lysosomes."),
          buildStorySlide("Initial condition",
              "The cell membrane has surface receptors that are exposed to the extracellular environment, ready to bind specific ligands (such as hormones, nutrients, or pathogens)."),
        ],
      ),
    );
  }

  // Build individual slide for Story Mode
// Build individual slide for Story Mode
  Widget buildStorySlide(String title, String description) {
    return Padding(
      padding: const EdgeInsets.all(23),
      child: Card(
        elevation: 5, // Adds shadow effect for the card
        color: Colors.teal[50],
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(12), // Rounded corners
        ),
        child: Padding(
          padding: const EdgeInsets.fromLTRB(16, 50, 16, 40),
          child: SingleChildScrollView(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.start,
              crossAxisAlignment: CrossAxisAlignment.center,
              children: [
                Text(
                  title,
                  style: const TextStyle(
                    fontSize: 25,
                    fontWeight: FontWeight.bold,
                    color: Colors.black,
                  ),
                  textAlign: TextAlign.center,
                ),
                if (currentStage != 4 && currentStage != 5)
                  const SizedBox(height: 30),
                Padding(
                  padding: const EdgeInsets.fromLTRB(18, 22, 18, 22),
                  child: ClipRRect(
                    borderRadius: BorderRadius.circular(
                        15.0), // Adjust the radius as needed
                    child: Image.asset(
                      'lib/assets/$currentStage.png', // Replace with your image URL
                      // width: 300,
                      fit: BoxFit.cover,
                    ),
                  ),
                ),
                if (currentStage != 4 && currentStage != 5)
                  const SizedBox(height: 30),
                Column(
                  children: [
                    Padding(
                      padding: const EdgeInsets.fromLTRB(14, 8, 14, 8),
                      child: Text(
                        description,
                        style: const TextStyle(fontSize: 20),
                        textAlign: TextAlign.start,
                      ),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
