# Quick Start Guide

Get up and running with Robot Path Planner in 5 minutes!

## Step 1: Build the Application

```bash
cd RobotPathPlanner
mkdir build && cd build
cmake ..
cmake --build .
```

## Step 2: Run the Application

```bash
./RobotPathPlanner
```

## Step 3: Create Your First Map

1. Click **"Draw Line"** tool in toolbar
2. Click and drag to create walls
3. Each line shows its length automatically
4. Create a simple rectangular field:
   - Draw 4 lines forming a rectangle (e.g., 10m x 5m)

## Step 4: Create a Path

1. In the **Paths** panel (right side), click **"New"**
2. Enter a name: "Test Path 1"
3. Click **"Draw Path"** tool
4. Click on the map to place waypoints
5. See the path length update in the Paths panel

## Step 5: Configure Robot Shape

1. In the **Robot** panel, select shape:
   - Rectangle (default for most FRC robots)
   - Square
   - Triangle
2. Adjust **Width**: 0.6 m
3. Adjust **Length**: 0.8 m
4. See robot preview on the map

## Step 6: Save Your Work

**Save the map:**
- File → Save Map (or Ctrl+S)
- Choose location and filename

**Save paths:**
- In Paths panel, click **"Save All Paths"**

## Step 7: Connect to Robot (Optional)

**If you have robot code running:**

1. Enter robot IP in Robot panel (e.g., `10.0.0.2`)
2. Click **"Connect"**
3. Once connected, you'll see:
   - Real-time robot position
   - Live heading updates
   - Distance to nearest wall

4. Send path to robot:
   - Select a path in Paths panel
   - Click **"Send to Robot"**

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+N   | New Map |
| Ctrl+O   | Open Map |
| Ctrl+S   | Save Map |
| Ctrl+R   | Send Path to Robot |
| Ctrl++   | Zoom In |
| Ctrl+-   | Zoom Out |
| F        | Fit to View |

## Tools Overview

| Tool | Description |
|------|-------------|
| **Select** | Select and edit existing elements |
| **Draw Line** | Click-drag to draw walls/boundaries |
| **Draw Path** | Click to add waypoints to current path |
| **Measure** | Click-drag to measure distances |
| **Pan** | Click-drag to move the view |

## Common Operations

### Zoom and Navigate
- **Mouse wheel**: Zoom in/out
- **Pan tool** + drag: Move view
- **F key**: Fit entire map in view
- **Reset button**: Return to default view

### Edit Map
- Grid size: Settings panel → adjust "Grid Size"
- Set origin: Settings panel → "Set Origin" button
- Delete line: Select tool → click line → Delete (not yet implemented in UI)

### Manage Paths
- **New path**: Paths panel → "New"
- **Delete path**: Select in list → "Delete"
- **Duplicate**: Select → "Duplicate"
- **Switch paths**: Click path name in list

### Measure Distance
1. Select **Measure** tool
2. Click start point
3. Drag to end point
4. Distance shown on line and in status bar

## Example Workflow

**Creating an autonomous path for FRC:**

1. **Create field map** (draw 27ft x 54ft field converted to meters: ~8.2m x 16.5m)
   ```
   - Draw outer boundaries
   - Draw any obstacles
   - File → Save Map
   ```

2. **Create autonomous path**
   ```
   - Paths → New → "Auto - Left Side"
   - Draw Path tool
   - Click starting position
   - Click intermediate points
   - Click ending position
   - Save All Paths
   ```

3. **Test on robot**
   ```
   - Deploy robot code (see examples/ folder)
   - Connect GUI to robot
   - Select path
   - Send to Robot
   - Watch robot move on map in real-time!
   ```

## Next Steps

- **Read full README.md** for detailed features
- **Check examples/README.md** for robot integration
- **Experiment with multiple paths** for different autonomous modes
- **Use measurement tool** to verify distances
- **Create field templates** for competition fields

## Troubleshooting

**Map not saving?**
- Check file permissions
- Try saving to Desktop or Documents folder

**Can't zoom?**
- Try using toolbar buttons instead of mouse wheel
- Check if mouse wheel scrolling is enabled in system settings

**Robot not connecting?**
- Verify robot is on and connected to network
- Check IP address is correct
- Ensure robot code is running TCP server on port 5800
- Check firewall settings

**Path looks wrong?**
- Verify origin is set correctly (Settings panel)
- Check coordinate system matches robot's
- Use Measure tool to verify distances

## Tips for Best Results

1. **Match grid size to your needs**: 1m for general use, 0.5m for detailed work
2. **Set origin first**: Before creating paths, set origin to match robot's coordinate system
3. **Use measurement tool**: Verify distances before sending paths to robot
4. **Save frequently**: Use Ctrl+S often to avoid losing work
5. **Test locally**: Create and visualize paths before deploying to robot
6. **Name paths clearly**: Use descriptive names like "Auto-Left-3Ball" vs "Path 1"

## What You've Learned

- ✓ Building and running the application
- ✓ Creating maps with walls
- ✓ Drawing paths with waypoints
- ✓ Configuring robot visualization
- ✓ Saving and loading maps/paths
- ✓ Connecting to robot (optional)
- ✓ Basic tools and shortcuts

Ready to dive deeper? Check out the full [README.md](README.md) and [examples/README.md](examples/README.md)!
