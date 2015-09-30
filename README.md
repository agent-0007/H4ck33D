# H4ck33D
H4ck33D – hacking a 433MHz Remote Control (http://mightydevices.com/)

Many of us live in so called ‘gated communities’ which are often surrounded by fences and/or walls built to provide to all residents some sense of security and isolation or (as it will be described later) an illusion of it. The access to such places is only possible through a limited number of dedicated gates which are often electronically controlled. Since I don’t possess a car, I wasn’t given a remote control which would allow me to open any of gates that lead to my house. Although it may not seem to be very problematic for a guy who still didn’t manage to get his own ride, but whenever any friend of mine (who often do have cars) wants to drop by, then he has to park his car outside which may sometimes be very inconvenient. As you can imagine, probably the best solution for this kind of situation would be to just ask some people who administer the spot, to give me one of those ‘cheap looking’ remote controls, but hey – where’s the fun in that? And so I’ve decided to hack my way into this system :)

Step 1. Gather all the tools you need

Almost all of those systems that are currently available on market use RF communication. I’ve never seen a gate opener that uses infra-red or any other communication medium. Many of those make use of ISM bands which in Europe are: 433MHz, 868MHz, 2.4GHz, 433MHz being the most popular one for such purposes. To be able to monitor ongoing communication one needs to have a receiver capable of tuning to 433MHz band and recording received signals on the go. There is no better tool for the job than SDR receiver. I’m using the most popular one (RTL-SDR), bought almost for peanuts, depicted below (plastic enclosure was removed for the ‘wow!’s).

  rtlsdr2

And now for the software, the one and only SDR#. User interface pretty much explains itself. Here you can see that my RTL SDR dongle pics up two FM broadcast stations around 105MHz. What’s most important: this marvelous piece of software allows you to tune into your band of interest and record baseband (or ‘received’ if you prefer)  signal in *.wav file format.

sdrsharp

So much for the reception. Now we need something to analyse incoming signals. I’ve used Audacity, as it works nicely with *.wav files produced by SDR#.

After capturing and analyzing incoming signals we will need some sort of radio transmitter that will behave just like the original remote control, and, for that matter, I’ve build my own from scratch.

Step 2. Capture

One needs to know what he is looking for, obviously. As I mentioned before jumping around 433MHz (433.82MHz to be exact, as it’s the most common SAW resonator frequency, often used in small remote controls) seems like a good idea. In my neighborhood there is quite a lot going on in that band, household weather stations, car alarms and  some sensor networks activity can be observed. Simple yet important question arises: How can I distinguish wanted signal from all that radio chatter? Well, answer won’t be straightforward, although there are some unique features that may indicate that we are right on spot:

Strong correlation to gate openings – doesn’t call for explanation :)
Signal strength – the closer to the gate you’ll get the stronger received signal should be. Just keep in mind that signal is not transmitted from the gate itself, but from remote control that is in use by people approaching the gate in their cars. Many remote controls produce a decent amount of RF power, so it’s possible to open the gate while still being distant. This is good, as you’ll probably won’t need to sit with your laptop and all RF tools in front of the object of your hacking, looking all suspicious and attracting unwanted attention :)
Here’s a screenshot of SDR# while receiving gate opening signal. (Care to know what WOW! signal is?)

wow

Step 3. Analyze

Quite a few things can be deduced from the picture above. First of all we can see that transmitter sends frames in periodic manner, and that all the frames being sent are of equal length. Small frequency drift can be observed. This leads to conclusion that transmitter does not use PLL nor any other form of hand effect (transmitter de-tuning caused by hand’s proximity) compensation. This indicates that we are dealing with some sort of cheap’n’simple circuitry. Needles to say – this is exactly what we were hoping for!

Using SDR# I’ve recorded baseband in the period during which transmission occured. Then, I’ve simply opened the recorded file in Audacity, and this is what I’ve got (after removing all moments of radio silence, etc.):

audacity

Well, look at that – a thing of beauty! Signal was so strong that I don’t even need to do any sort of processing, as one can see all the bits, 1’s and 0’s with his naked eye. We can clearly see that we are about to deal with OOK modulation scheme – needless to say, it’s the simplest modulation of all.

Every transmitted frame was always the same – this yields to conclusion that we are dealing with one-way communication here. This is to be expected from such a sleazy system, and it’s very good news. We won’t have to understand the encoding used, as we can simply mimic transmitter’s behavior, and the gate opening system won’t even notice that someone’s messing with it.

First part of this waveform is, so called ‘preamble’, and it consists of alternating pattern of ones and zeros. It is often used in radio communication as it helps the receiver to synchronize with transmitters clock. In the picture above 24 bits (twelve zeros and twelve ones) can be observed. Then there is a moment of silence, purpose of which still remains a mystery to me. Silence is followed by a specific bit pattern. What’s so specific about it? Let’s dive in into details:

symbols

We are dealing with two kinds of symbols here, just like in Morse Code: the short one (dit), and the long one (dash). Careful observer will notice that transmitting short symbol requires just as much time as the long one (time intervals between thick lines are always equal to each other):

symbols2

Every symbol’s interval is divided into three equal parts, and short symbol is transmitted in such way that carrier is on during first part, and the long symbol takes up to two parts of carrier activity. This is done to prevent constant carrier presence (or lack of) when one would like to transmit a long sequence of zeros or ones. Such approach gives the receiver possibility to re-synchronize after every symbol received at the expense of bit rate (symbol is build up of three ‘bits’ instead of one).

Let’s assume that short symbol represent binary zero, and the long one represent binary one. This assumption may be wrong, but it won’t matter in the end, as I am not planning to reverse-engineer every bit of every frame here. No need for that.

Important signal parameters that one should write down:

bit rate – duration of a single ‘bit’ – 380us
preamble length – 24 bits (1010101….)
radio ‘silence’ length – 9 bits (funny number, I know, but there is more funny numbers to come..)
bit sequence – sequence of dits and dashes…
interval between emitted frames – 105 ms
Having all the information needed to reproduce received signal, we can finally hit the radio waves!

Step 4. Build a transmitter

Since we are dealing with OOK modulation, I’ve managed to design and finally construct a simple 433.82MHz transmitter with modulation input. See the schematic below.

433txsch

Presented circuit can be divided into two parts: a Colpitts oscillator with MOD signal (that turns on/off oscillation) built around Q1 and stabilized with SAW resonator, and a simple power amplifier based on Q2. I’ve powered whole thing from 5V DC, and observed SDR# waterfall diagram when MOD pin was tied to VCC and then to GND. Here’s the whole thing soldered on small piece of PCB:

433tx

Modulation will be done by STM32L152 on DISCOVERY board. Here’s a picture showing a transmitter connected to DISCOVERY:

433tx2

Step 5. Hack your way into gated community

I’ve written simple code that will periodically send frames that I’ve previously collected. I am currently using three different payloads for three gates, all o which seem to work. What code does can be summarized as:

generating bit stream on pin PA12 – this is done in Timer6 interrupt to keep the timing tight!
encoding frames – changing symbols into ‘dits’ and ‘dashes’, preparing preamble
preparing one frame every 105 ms.
Resulting binary occupies around 3kB. Needless to say – it works outstandingly well.
