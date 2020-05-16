from flask import Flask, render_template, redirect, Response, send_file
import os
import mimetypes
mimetypes.add_type('application/wasm', '.wasm', strict=True)
port = int(os.environ['PORT'])
base_url = os.environ['BASE_URL']
app = Flask(__name__, static_folder='.', static_url_path=base_url)

@app.route(base_url)
def get_index():
    return send_file('index.html')

@app.errorhandler(404)
def error_404(e):
    return redirect(base_url)

if __name__ == '__main__':
    app.run(debug=False, port=port)
