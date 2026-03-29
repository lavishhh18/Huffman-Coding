import os
import glob
import subprocess
from flask import Flask, render_template, request, send_file, abort

# Configure Application
app = Flask(__name__)

filename = ""
ftype = ""

@app.route("/")
def home():

    # Delete old files
    filelist = glob.glob('uploads/*')
    for f in filelist:
        os.remove(f)
    filelist = glob.glob('downloads/*')
    for f in filelist:
        os.remove(f)
    global filename
    global ftype
    filename = ""
    ftype = ""
    return render_template("home.html")

app.config["FILE_UPLOADS"] = "uploads"

@app.route("/compress", methods=["GET", "POST"])
def compress():

    if request.method == "GET":
        return render_template("compress.html", check=0)

    else:
        up_file = request.files["file"]

        if len(up_file.filename) > 0:
            global filename
            global ftype
            upload_name = os.path.basename(up_file.filename)
            upload_path = os.path.join(app.config["FILE_UPLOADS"], upload_name)
            up_file.save(upload_path)

            subprocess.run(["./c", upload_path], check=True)

            filename = upload_name.split(".", 1)[0]
            ftype = "-compressed.bin"
            compressed_path = os.path.join("uploads", f"{filename}{ftype}")
            if not os.path.exists(compressed_path):
                raise FileNotFoundError(compressed_path)
            os.replace(compressed_path, os.path.join("downloads", f"{filename}{ftype}"))

            return render_template("compress.html", check=1)

        else:
            return render_template("compress.html", check=-1)

@app.route("/decompress", methods=["GET", "POST"])
def decompress():

    if request.method == "GET":
        return render_template("decompress.html", check=0)

    else:
        up_file = request.files["file"]

        if len(up_file.filename) > 0:
            global filename
            global ftype
            upload_name = os.path.basename(up_file.filename)
            upload_path = os.path.join(app.config["FILE_UPLOADS"], upload_name)
            up_file.save(upload_path)

            subprocess.run(["./d", upload_path], check=True)

            filename = upload_name.split("-compressed", 1)[0]
            matches = glob.glob(os.path.join("uploads", f"{filename}-decompressed.*"))
            if not matches:
                raise FileNotFoundError(f"decompressed output for {upload_name} not found")

            decompressed_name = os.path.basename(matches[0])
            if "-decompressed." not in decompressed_name:
                raise ValueError(f"unexpected decompressed filename: {decompressed_name}")
            ftype = "-decompressed." + decompressed_name.split("-decompressed.", 1)[1]
            os.replace(matches[0], os.path.join("downloads", f"{filename}{ftype}"))

            return render_template("decompress.html", check=1)

        else:
            return render_template("decompress.html", check=-1)






@app.route("/download")
def download_file():
    global filename
    global ftype
    if not filename or not ftype:
        abort(404)
    path = "downloads/" + filename + ftype
    if not os.path.exists(path):
        abort(404)
    return send_file(path, as_attachment=True)




# Restart application whenever changes are made
if __name__ == "__main__":
    app.run(debug = True)
