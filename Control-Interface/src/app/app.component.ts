import {Component, ElementRef, OnInit, ViewChild} from '@angular/core';
import { CommonModule } from '@angular/common';
import { RouterOutlet } from '@angular/router';
import {HttpClient, HttpClientModule, HttpHeaders} from "@angular/common/http";

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [CommonModule, RouterOutlet, HttpClientModule],
  templateUrl: './app.component.html',
  styleUrl: './app.component.scss'
})
export class AppComponent implements OnInit {
  @ViewChild("canvas", {static: true}) canvas!: ElementRef<HTMLCanvasElement>;
  public ctx: CanvasRenderingContext2D;

  public images: string[]
  private intervals: number[]
  private timeouts: number[]

  constructor(private http: HttpClient) {
    this.images = []
    this.intervals = []
    this.timeouts = []
  }

  ngOnInit(): void {
    this.ctx = <CanvasRenderingContext2D>this.canvas.nativeElement.getContext("2d");
  }

  handleFileInput($event: any) {
    this.images = []

    for (let i = 0; i < $event.target.files.length; i++) {
      const fileToUpload = $event.target.files.item(i)

      let reader = new FileReader();
      reader.onload = (event: any) => {
        const img = new Image();
        img.src = event.target.result;
        img.onload = () => {
          this.ctx.drawImage(img, 0, 0, 40, 32)
          const imgData = this.ctx.getImageData(0, 0, 40, 32).data;

          this.images.push(this.getRGB565Complete(imgData))
        }
      };
      reader.readAsDataURL(fileToUpload);
    }

    this.timeouts.push(setTimeout(() => this.play(), 1000))
  }

  randomImage() {
    this.images = []
    const img = new Image();
    img.crossOrigin = "Anonymous";
    img.src = "https://picsum.photos/40/32";
    img.onload = () => {
      this.ctx.drawImage(img, 0, 0, 40, 32)
      const imgData = this.ctx.getImageData(0, 0, 40, 32).data;

      this.images.push(this.getRGB565Complete(imgData))

      this.play()
    }
  }

  readonly TIMEOUT = 1000

  private play() {
    this.intervals.forEach(interval => window.clearInterval(interval))
    this.timeouts.forEach(timeout => window.clearInterval(timeout))

    if (this.images.length === 1) {
      this.sendRequest(this.images[0])
    } else if (this.images.length > 1) {
      this.intervals.push(
        setInterval(() => {
          this.images.forEach((e, i, a) => {
            this.timeouts.push(
              setTimeout(() => this.sendRequest(e), i * this.TIMEOUT)
            )
          })
        }, this.images.length * this.TIMEOUT)
      )
    }
  }

  private getRGB565Complete(imgData: Uint8ClampedArray): string {
    let output = ""
    for (let i = 0; i < 5120; i = i + 4) {
      output += (this.getRGB565Single(imgData[i], imgData[i + 1], imgData[i + 2]))
    }
    return output
  }

  private getRGB565Single(r: number, g: number, b: number): string {
    const red = r.toString(2).padStart(8, "0").substr(0, 5)
    const green = g.toString(2).padStart(8, "0").substr(0, 6)
    const blue = b.toString(2).padStart(8, "0").substr(0, 5)
    return parseInt(red + green + blue, 2).toString(16).padStart(4, "0").toUpperCase();
  }

  private sendRequest(image565: string) {
    this.http.post('/handleFileUpload', image565, {responseType: 'text'}).subscribe(value => console.log(value))
  }
}

