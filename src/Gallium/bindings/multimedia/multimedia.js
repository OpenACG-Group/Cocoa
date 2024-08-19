// %scope UserExecute:forbidden UserImport:forbidden SysExecute:allowed

/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */
'use strict';

(function (natives){
    function gcd(m, n) {
        while (n !== 0) {
            const r = m % n;
            m = n;
            n = r;
        }
        return m;
    }

    //! @tsdocbegin
    //! Immutable object, represents a rational number x (x ∈ ℚ) accurately, where x = num / den.
    //! Basic arithmetic calculations are supported.
    //!
    //! While rational numbers can be expressed as floating-point numbers, the conversion process
    //! is a lossy one, so are floating-point operations. On the other hand, the nature of multimedia
    //! demands highly accurate calculation of timestamps. This set of rational number utilities
    //! serves as a generic interface for manipulating rational numbers as pairs of numerators and
    //! denominators.
    //! @tsdocend
    //! TSDecl: @class Rational
    class Rational {
        #num
        #den

        //! @tsdocbegin
        //! Constructs a rational number from a pair of specified numerator and denominator.
        //! Both `num` and `den` must be integers, and `den` must not be zero.
        //! Note that the given fraction will be reduced (simplified):
        //!     new Rational(20, 10) => 2:1
        //!     new Rational(-20, -10) => 2:1
        //!     new Rational(0, 100) => 0:1
        //!     ...
        //!
        //! If the fraction is negative, sign is carried by the numerator.
        //! @tsdocend
        //! TSDecl: @constructor(num: i32, den: i32)
        constructor(num, den) {
            if (!Number.isInteger(num) || !Number.isInteger(den)) {
                throw RangeError('numerator and denominator must be integers');
            }
            if (den === 0) {
                throw RangeError('denominator must not be zero');
            }
            const sign = num * den > 0 ? 1 : -1;
            this.#num = Math.abs(num) * sign;
            this.#den = Math.abs(den);
            this.#reduce();
        }

        #reduce() {
            if (this.#den === 1) {
                return;
            }
            const k = gcd(Math.abs(this.#num), this.#den);
            this.#num /= k;
            this.#den /= k;
        }

        //! TSDecl: @property @readonly num: i32
        get num() {
            return this.#num;
        }

        //! TSDecl: @property @readonly den: i32
        get den() {
            return this.#den;
        }

        //! TSDecl: @method toString(delimiter: string): string
        toString(delimiter = ':') {
            return `${this.#num}${delimiter}${this.#den}`;
        }

        //! TSDecl: @method add(r: Rational): Rational
        add(r) {
            return new Rational(
                this.#num * r.#den + r.#num * this.#den,
                this.#den * r.#den
            );
        }

        //! TSDecl: @method sub(r: Rational): Rational
        sub(r) {
            return new Rational(
                this.#num * r.#den - r.#num * this.#den,
                this.#den * r.#den
            );
        }

        //! TSDecl: @method mul(r: Rational): Rational
        mul(r) {
            return new Rational(this.#num * r.#num, this.#den * r.#den);
        }

        //! TSDecl: @method scale(x: f64): f64
        scale(x) {
            return x / this.#den * this.#num;
        }

        //! TSDecl: @method inv(): Rational
        inv() {
            if (this.#num === 0) {
                throw Error('arithmetic error: divide by zero');
            }
            return new Rational(this.#den, this.#num);
        }

        //! TSDecl: @method div(r: Rational): Rational
        div(r) {
            return new Rational(this.#num * r.#den, this.#den * r.#num);
        }

        //! TSDecl: @method toFloat64(): f64
        toFloat64() {
            return this.#num / this.#den;
        }
    }
    //! TSDecl: @end

    //! TSDecl: @interface FrameIterationResult
    //! TSDecl: @property mediaType: MediaType
    //! TSDecl: @property frame: Frame
    //! TSDecl: @end

    //! TSDecl: @interface FrameIterationOptions
    //! @tsdocbegin
    //! Called only once when creating a codec context for an audio or video stream.
    //! @tsdocend
    //! TSDecl: @property @optional setAdditionalCodecOptions:
    //! TSDecl:         @fn(void, streamInfo: DemuxStreamInfo, builder: CodecContextBuilder)
    //! TSDecl: @end

    //! @tsdocbegin
    //! A helper function that reads packets from the specified demuxer, decodes them automatically,
    //! and returns decoded frames in each iteration. It has the same effect to manually creating
    //! codec contexts and sending, receiving frames, but more convenient for simple situations.
    //!
    //! The function itself is not a generator function, but it returns an iterable `Generator` object,
    //! which is equivalent to the result of calling a generator function.
    //!
    //! Note that the yielded `Frame` instance keeps valid until the next iteration.
    //! @tsdocend
    //! TSDecl: @function IterateMediaFrames(demuxer: FormatDemuxer, options: FrameIterationOptions)
    //! TSDecl:     : @generic(Generator, FrameIterationResult, FrameIterationResult)
    function IterateMediaFrames(demuxer, options) {
        const videoIdx = demuxer.findBestStream(natives.MediaType.Video);
        const audioIdx = demuxer.findBestStream(natives.MediaType.Audio);

        let videoStreamInfo = null;
        let videoCodecCtx = null;
        if (videoIdx >= 0) {
            videoStreamInfo = demuxer.getStreamInfo(videoIdx);
            const builder = new natives.CodecContextBuilder(natives.CodecType.Decoder);
            builder.setCodecParameters(demuxer.getCodecParameters(videoIdx))
            builder.setPacketTimebase(videoStreamInfo.timeBase);
            options?.setAdditionalCodecOptions?.(videoStreamInfo, builder);
            videoCodecCtx = builder.detach();
        }

        let audioStreamInfo = null;
        let audioCodecCtx = null;
        if (audioIdx >= 0) {
            audioStreamInfo = demuxer.getStreamInfo(audioIdx);
            const builder = new natives.CodecContextBuilder(natives.CodecType.Decoder);
            builder.setCodecParameters(demuxer.getCodecParameters(audioIdx))
            builder.setPacketTimebase(audioStreamInfo.timeBase);
            options?.setAdditionalCodecOptions?.(audioStreamInfo, builder);
            audioCodecCtx = builder.detach();
        }

        return (function*() {
            let packet = null;
            let frame = null;

            while (true) {
                packet = demuxer.readFrame(packet);
                if (packet == null) {
                    break;
                }

                let decoder = packet.streamIndex === audioIdx ? audioCodecCtx : videoCodecCtx;
                let status = decoder.sendPacket(packet);

                if (status !== natives.CodecStatus.Success) {
                    throw Error('failed to send packets to the decoder');
                }

                while (true) {
                    [status, frame] = decoder.receiveFrame(frame);
                    if (status === natives.CodecStatus.NeedFeedInput) {
                        break;
                    }
                    if (status !== natives.CodecStatus.Success) {
                        throw Error('failed to receive frames from the decoder');
                    }

                    yield {
                        mediaType: packet.streamIndex === audioIdx
                            ? natives.MediaType.Audio : natives.MediaType.Video,
                        frame: frame
                    };

                    frame.disposeReusable({});
                }
                packet.disposeReusable();
            }

            audioCodecCtx?.dispose();
            videoCodecCtx?.dispose();

            return {
                mediaType: natives.MediaType.Unknown,
                frame: null
            };
        })();
    }

    // Implmentations of some helper functions
    natives.Frame.MakeAllocated = function (spec) {
        const frame = natives.Frame.MakeUnallocated(spec);
        frame.allocate();
        return frame;
    };

    return {
        Rational: Rational,
        IterateMediaFrames: IterateMediaFrames
    };
});
