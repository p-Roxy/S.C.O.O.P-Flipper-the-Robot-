# Flipper the Robot
Patrick Coleman, Jack Mendeville, Caleb Chuang, Jack Chen, Roxy Promhouse

Used an Arduino and Lego Mindstorms to create a small, autonomous robot in a group of 5.

Final Write-Up Report:<br>
Strategy- worked/didn’t work<br>
S.C.O.O.P.’s code has two main parts.  The first phase of the robot’s strategy is to charge forward a set distance, approximately enough to reach the centre of the ring.  After that, the robot will spin clockwise until it detects another robot in front of it. Once the robot senses something, it will drive towards it. The robot will then raise and lower the scoop once it is close enough. If the distance sensor stops sensing an object then the robot will resume the spinning behavior.

The spinning worked well, same as last time. Even though we added the drive forward behavior, the robot never drove itself out of the ring because it would only drive forward if there was another robot in front of it.  The lack of a sensor for the edge of the ring did mean we had no way to avoid being pushed out of the ring by other robots, though.

The scoop worked a lot better this time. We think that adding the drive forward behavior before scooping allowed the scoop to be much more effective. In our last match, we successfully scooped another robot for the K.O.

<br>Challenges<br>
Most of our challenges were the same as last week.  For example, our calibration of the scoop’s distance sensor wasn’t perfect, and there were times when it was facing another robot but did not raise the scoop.

In addition to the scoop being able to push too far down (a challenge we faced last week), we also found it would push itself too far up and smack the robot body, sometimes hitting the sensors.  We were able to implement pieces to block the scoop from raising too far.

We also faced a few new challenges that we did not encounter last week. Several times our wires came out overnight, so we had to be extra careful to put them in the correct place and reinforce them. This was also complicated by the fact that the Meme Machine was working this week and their main strategy is to pull wires with their spinning propellor.  We did replace some of the parts at the back of the robot, as some wires were being pulled too far (due to the size of the robot) and were more liable to fall out.

<br>Hardest Competitor<br>
Our hardest competition last week was Carl. This week, however, Carl was not working due to motor relay difficulties so we were able to easily beat it in our matches against it.

Our hardest competitor in the second round was the Meme Machine.  Our battles with it frequently ended in the two robots getting stuck trying to push each other, often with various extended parts of both robots getting tangled.  In the final round the scoop was able to tilt Meme Machine over for a decisive victory.  However, there was a match where they were able to push our robot out of the ring.  

The other robots for the most part drove themselves out of the rings without any offense on our parts, and our spinning code worked well enough to give us the victory in these cases.  As a result we were not as challenged by the other competitors.

<br>Changes from Last Tournament<br>
Since last week we added pieces to the scoop to prevent it from scooping too far and hitting itself. We think this helped mitigate the risk of our robot damaging itself.

Additionally, we improved our battle strategy by adding the drive forward behavior before the robot scooped. We think this helped us in battle a lot because it enabled our robot to go on the offensive instead of just waiting to be attacked by the opponent.

Though last week we had questioned whether or not our scoop was an effective weapon (as it did not seem to have enough power to provide any use as a weapon), this week it tilted opponents’ robots on several occasions and won the final match for us by flipping Meme Machine over.  Keeping the scoop (with the changes in coding and the extra pieces) proved to be a good choice.

We also added the ability for our robot to play music while in battle. We believe this helped to improve morale and make the opponents scared of our robot. However, the music was hard to hear at times because the motors required a lot of power from the batteries and so the speaker did not get enough power to play loudly.

Having learned from last week’s battles, we also added parts of lego that mimicked strengths from lab opponents (a small wall of blocks like Carl, protective tubing like GJPMMDTM2K16, and a long arm like Bernie). While the wall and tubing were mostly decorative, the arm did not bring much benefit and blocked some opportunities for scooping, so was later removed.
