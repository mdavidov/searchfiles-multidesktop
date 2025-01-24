#include "aboutdialog.h"
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>


AboutDialog::AboutDialog(QWidget* parent /*= nullptr*/) : QDialog(parent) {
    setWindowTitle("About");
    setModal(true);
    resize(600, 540);  // Set a reasonable default size

    QVBoxLayout* layout = new QVBoxLayout(this);

    QTextBrowser* htmlBrowser = new QTextBrowser;
    htmlBrowser->setOpenExternalLinks(true);
    htmlBrowser->setHtml(R"(
        <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
        <html xmlns="http://www.w3.org/1999/xhtml">
        <head>
        <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
        <title>About getinfo</title>
        <!--<link href="style.css" rel="stylesheet" type="text/css"/>-->
        </head>
        <body>
        <div id="main_content" align="center">
        <div id="content"  align="center">
        <!--<p><img src="images/getinfo.png"</img></p>-->
        <br/>
        <h2><a href="http://devonline.au">devonline.au</a></h2>
        <br/>
        <p align="center"><strong>Copyright &copy; 2012-2025 Milivoj (Mike) Davidov. All rights reserved. </strong></p>
        <br/>
        <p align="left">
        This software application was developed with best intentions; the author believes it functions correctly.
        <br/>
        However:
        <br/>
        <br/>
        THIS SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESSED
        OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
        FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
        <br/>
        <br/>
        IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
        DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
        OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
        USE OR OTHER DEALINGS IN THE SOFTWARE.
        <br/>
        <br/>
        WARNING: This computer program is protected by copyright law and international treaties.
        </p>
        <!--
        <p align="center">Favourite quote or slogan.</p>
        <h2>H2 SubTitle #2</h2>
        <p>Time flies when you are having fun. </p>
        -->
        </div>
        </div>
        <!--<div id="footer" align="center"><strong>Copyright &copy; Milivoj (Mike) Davidov</strong></div>-->
        <br/>
        </body>
        </html>
    )");

    QPushButton* closeButton = new QPushButton("Close");

    layout->addWidget(htmlBrowser);
    layout->addWidget(closeButton);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}
