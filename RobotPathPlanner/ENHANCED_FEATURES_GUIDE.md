# Enhanced Features - Complete Guide

## 🎉 All Your Requested Features Are Now Implemented!

### ✅ Feature Checklist:
- ✅ **Visual waypoint heading control** - Drag the arrow to rotate
- ✅ **Heading displayed in degrees** - Shows on each waypoint
- ✅ **Drag & drop robot placement** - Click and drag robots anywhere
- ✅ **Multiple robots on same map** - Add as many as you need
- ✅ **Reference points tool** - Now in toolbar with "Add Ref" button

---

## 1️⃣ Control Waypoint Heading Visually

### How to Rotate Waypoint Heading:

**Method 1: Drag the Arrow (NEW!)**
1. Select the **"Select"** tool from toolbar
2. Click and drag the **arrow tip** at the end of any waypoint
3. The arrow rotates as you move your mouse
4. The heading updates in real-time
5. Release to set the heading
6. **Heading in degrees** is displayed below the waypoint

**Method 2: Double-Click to Edit (Precise)**
1. Double-click any waypoint marker
2. Enter exact heading in degrees
3. See visual preview of direction
4. Click OK

### Visual Indicators:
- **Large arrow** shows waypoint heading direction
- **Arrowhead** points in travel direction
- **Degrees label** shows angle (e.g., "45°", "90°", "-90°")
- **Real-time update** as you drag

**Example angles:**
- `0°` = Right/East
- `90°` = Up/North
- `180°` or `-180°` = Left/West
- `-90°` = Down/South

---

## 2️⃣ Heading in Degrees

### Degree Display:
Every waypoint now shows its heading in degrees **automatically**:

```
Waypoint Marker (blue circle)
      ↓
   ════► Arrow points direction
      ↓
    [45°]  ← Heading in degrees
```

### Reading the Display:
- **White background** for easy visibility
- **Bold text** in the path color
- **Integer degrees** (no decimal places)
- **Normalized** to -180° to 180° range

### In JSON Export:
```json
{
  "waypoints": [
    {
      "x": 2.5,
      "y": 3.0,
      "heading": 45.0,    ← In degrees for readability
      "velocity": 1.5
    }
  ]
}
```

**Note**: The GUI shows degrees, but the robot code receives radians (auto-converted).

---

## 3️⃣ Drag & Drop Robot Placement

### How to Move Robots:

**Quick Method:**
1. Make sure **"Select"** tool is active (or no tool selected)
2. **Click on any robot** on the map
3. **Drag** to new position
4. **Release** to drop
5. Status bar shows current position

**Visual Feedback:**
- Cursor changes to **closed hand** when dragging
- Robot follows mouse in real-time
- Position displayed in status bar: `"Robot 1 at X: 2.345 m, Y: 1.234 m"`

**Tips:**
- ✓ Robots can be dragged anywhere on the map
- ✓ No need to switch tools - just drag!
- ✓ Works with multiple robots
- ✓ Right-click to cancel drag

---

## 4️⃣ Multiple Robots on Same Map

### Adding Robots:

**Step 1: Add a Robot**
1. Open **"Robot"** panel (right side)
2. Scroll to **"Multiple Robots"** section
3. Click **"Add Robot"** button
4. New robot appears at position (1.0, 1.0)

**Step 2: Position the Robot**
1. **Drag the new robot** to desired position
2. Adjust size/shape if needed
3. Repeat to add more robots

**Step 3: Manage Robots**
- **Robot count** shown in panel: "Robots: 3"
- **Remove Last** button removes newest robot
- **Cannot remove** the last robot (minimum 1)

### Primary Robot:
- The **PRIMARY** robot is labeled on the map
- This is the robot that receives NetworkTables updates
- When multiple robots exist, "PRIMARY" label appears in red

### Visual Differences:
- **All robots** are drawn on the map
- **Primary robot** has "PRIMARY" label above it
- **Each robot** can be dragged independently
- **Same shape/size** for all (for now)

### Use Cases:
✅ **Multi-robot autonomous** - Plan coordinated paths
✅ **Practice scenarios** - Show defender + attacker positions
✅ **What-if analysis** - Test different starting positions
✅ **Team coordination** - Visualize multiple robots on field

---

## 5️⃣ Reference Points on Map

### Adding Reference Points:

**Step 1: Select Tool**
1. Click **"Add Ref"** button in toolbar
2. Cursor changes to crosshair

**Step 2: Place Reference**
1. **Click anywhere** on the map
2. Reference point appears as **magenta diamond**
3. Auto-named as "Ref 1", "Ref 2", etc.
4. **Snaps** to nearby points if close

**Step 3: Use References**
- Reference points save with map
- Export to JSON with name and position
- Can optionally include heading

### Visual Appearance:
```
    ◆  ← Magenta diamond shape
  "Ref 1" ← Name label
```

### Reference Point Features:
- **Diamond marker** - Easy to see
- **Magenta color** - Stands out from paths
- **Named** - Identifies each reference
- **Optional heading** - Can store direction
- **Snappable** - Lines and paths can snap to them
- **Saved with map** - Export to JSON

### Use Cases:
✅ **Starting positions** - Mark where robot begins autonomous
✅ **Scoring locations** - Mark goals, baskets, targets
✅ **Loading zones** - Mark game piece pickup spots
✅ **Navigation waypoints** - Create network of key positions
✅ **Coordinate system** - Define field reference points

---

## 🎯 Complete Workflow Examples

### Example 1: Create Path with Precise Headings

```plaintext
1. Draw Path
   - Select "Draw Path" tool
   - Click waypoints on map

2. Set Headings Visually
   - Select "Select" tool
   - Drag each waypoint arrow to desired angle
   - Watch degrees update in real-time

3. Fine-Tune (Optional)
   - Double-click waypoint
   - Enter exact degrees
   - Set velocity

4. Export
   - "Save All Paths"
   - JSON includes X, Y, θ in degrees
```

### Example 2: Multi-Robot Scenario

```plaintext
1. Add Robots
   - Robot panel → "Add Robot" (click 2 times)
   - Now have 3 robots total

2. Position Robots
   - Drag Robot 1 to start position A
   - Drag Robot 2 to start position B
   - Drag Robot 3 to start position C

3. Add Reference Points
   - "Add Ref" tool → Click scoring zone
   - "Add Ref" tool → Click loading zone
   - Reference points mark key locations

4. Create Paths
   - Path 1: Robot 1 to scoring zone
   - Path 2: Robot 2 to loading zone
   - Each path can have different headings

5. Visualize
   - All robots visible on map
   - All paths visible
   - Reference points mark destinations
```

### Example 3: Autonomous Path Planning

```plaintext
1. Draw Field
   - "Draw Line" to create walls
   - Lines show angle (e.g., "90°", "45°")
   - Snap to corners automatically

2. Add Start Position
   - "Add Ref" → Click robot start position
   - Name it "Start Position"

3. Create Path
   - "Draw Path" from start reference
   - Add waypoints
   - Drag arrows to set heading at each point
   - Headings show in degrees

4. Set Robot
   - Drag robot to start position
   - Match robot heading to first waypoint

5. Export Everything
   - Save Map (includes walls + references)
   - Save Paths (includes X, Y, θ)
   - Load in robot code
```

---

## 🔧 Tips & Tricks

### For Waypoint Headings:
- 📍 **Drag arrow tip** for visual control
- 📍 **Double-click** for exact degrees
- 📍 **Degrees shown** automatically
- 📍 **Large arrows** for better visibility

### For Robot Placement:
- 🤖 **Just drag** - no tool needed!
- 🤖 **Status bar** shows position
- 🤖 **Multiple robots** supported
- 🤖 **Primary** robot gets network updates

### For Reference Points:
- 💎 **"Add Ref" tool** in toolbar
- 💎 **Magenta diamonds** stand out
- 💎 **Auto-snap** to nearby points
- 💎 **Saved with map** automatically

### General:
- 💡 **Select tool** enables dragging + heading editing
- 💡 **Right-click** cancels any operation
- 💡 **Snap enabled** by default (lines connect perfectly)
- 💡 **JSON export** includes all features

---

## 📊 JSON Export Format

### Complete Path with Headings:
```json
{
  "name": "Auto Path 1",
  "waypoints": [
    {
      "x": 1.5,
      "y": 2.0,
      "heading": 0.0,      // 0° = right
      "velocity": 1.5
    },
    {
      "x": 3.0,
      "y": 4.0,
      "heading": 90.0,     // 90° = up
      "velocity": 2.0
    },
    {
      "x": 5.0,
      "y": 4.0,
      "heading": 180.0,    // 180° = left
      "velocity": 1.0
    }
  ]
}
```

### Map with Reference Points:
```json
{
  "name": "Competition Field",
  "lines": [...],
  "referencePoints": [
    {
      "position": {"x": 2.0, "y": 1.0},
      "name": "Start Position",
      "heading": 90.0,
      "hasHeading": true
    },
    {
      "position": {"x": 8.0, "y": 3.0},
      "name": "Scoring Zone",
      "hasHeading": false
    }
  ]
}
```

---

## 🚀 Quick Reference Card

| **Feature** | **How To** | **Tool Required** |
|------------|-----------|------------------|
| **Rotate waypoint heading** | Drag arrow tip | Select tool |
| **Set exact heading** | Double-click waypoint | Any |
| **Move robot** | Click and drag robot | Select/None |
| **Add robot** | Robot panel → "Add Robot" | N/A |
| **Add reference point** | Click on map | "Add Ref" tool |
| **See heading degrees** | Automatic on waypoints | N/A |
| **Connect lines** | Draw near endpoints | "Draw Line" (auto-snap) |
| **Cancel operation** | Right-click | Any |

---

## ⌨️ Keyboard Shortcuts

| Key | Action |
|-----|--------|
| **Ctrl+S** | Save Map |
| **Ctrl+O** | Open Map |
| **Ctrl+N** | New Map |
| **Ctrl+R** | Send Path to Robot |
| **F** | Fit to View |
| **Ctrl++** | Zoom In |
| **Ctrl+-** | Zoom Out |
| **Esc** | Cancel Operation (TODO) |

---

## 🐛 Troubleshooting

**Q: Can't rotate waypoint heading**
- A: Switch to "Select" tool first
- A: Make sure you're dragging the arrow TIP, not the waypoint dot

**Q: Can't drag robot**
- A: Switch to "Select" tool or clear any active tool
- A: Make sure you're clicking ON the robot (green shape)

**Q: Reference points not appearing**
- A: Click "Add Ref" button in toolbar first
- A: Check that map panel is visible

**Q: Multiple robots all moving together**
- A: Only the PRIMARY robot receives network updates
- A: Other robots are manually positioned

**Q: Heading degrees not showing**
- A: Make sure path is visible (check Paths panel)
- A: Zoom in if text is too small

---

## 📖 For Robot Code

### Reading Headings from JSON:

```cpp
// In your robot code
QJsonObject pathJson = loadPath("path.json");

for (const auto& wpValue : pathJson["waypoints"].toArray()) {
    double x = wpValue["x"].toDouble();
    double y = wpValue["y"].toDouble();

    // Heading is in degrees in JSON
    double headingDeg = wpValue["heading"].toDouble();

    // Convert to radians for robot code
    double headingRad = headingDeg * M_PI / 180.0;

    // Use in trajectory
    addPose(x, y, headingRad);
}
```

### Using Reference Points:

```cpp
// Load reference points from map
QJsonObject mapJson = loadMap("map.json");

for (const auto& refValue : mapJson["referencePoints"].toArray()) {
    QString name = refValue["name"].toString();
    double x = refValue["position"]["x"].toDouble();
    double y = refValue["position"]["y"].toDouble();

    if (name == "Start Position") {
        setRobotStartPose(x, y);
    }
}
```

---

## ✨ What's New Summary

| Feature | Before | Now |
|---------|--------|-----|
| **Waypoint Heading** | Double-click only | Drag arrow OR double-click |
| **Heading Display** | Hidden | Shows in degrees on map |
| **Robot Position** | Code sets it | Drag anywhere manually |
| **Multiple Robots** | One robot only | Unlimited robots |
| **Reference Points** | Not available | Click to add, saves with map |
| **Line Angles** | Not shown | Shows while drawing |
| **Point Snapping** | Manual | Automatic |

---

## 🎓 Learn By Doing

### Tutorial: Complete Path with Headings

1. **Start**: Open the application
2. **Add Ref**: Click "Add Ref", place start marker
3. **Add Ref**: Place end marker
4. **New Path**: Paths panel → "New"
5. **Draw Path**: Click "Draw Path" tool
6. **Add waypoints**: Click 4-5 points from start to end
7. **Select tool**: Click "Select" in toolbar
8. **Rotate headings**: Drag each waypoint's arrow tip
9. **Check degrees**: See heading below each waypoint
10. **Save**: "Save All Paths"

**You now have a complete path with X, Y, and Theta!**

---

## 📝 All Features Implemented!

✅ **Control waypoint heading on map** - Drag arrows to rotate
✅ **Heading shown in degrees** - Automatic display
✅ **Drag & drop robots** - Click and drag anywhere
✅ **Multiple robots** - Add as many as needed
✅ **Reference points** - "Add Ref" button in toolbar

**Everything you requested is working! 🎉**

Enjoy your enhanced Robot Path Planner!
