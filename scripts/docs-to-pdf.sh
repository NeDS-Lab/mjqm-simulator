#!/bin/bash

mkdir -p $(pwd)/output/docs

docker run --rm -it \
  --cap-add=SYS_ADMIN \
  --user $(id -u):$(id -g) \
  -v $(pwd)/docs:/home/node/docs:ro \
  -v $(pwd)/output/docs:/home/node/pdf:rw \
  -v $(pwd)/.github/docsify-footnote.min.js:/home/node/resources/js/docsify-footnote.min.js:ro \
  -v $(pwd)/.github/prism-c.min.js:/home/node/resources/js/prism-c.min.js:ro \
  -v $(pwd)/.github/prism-cpp.min.js:/home/node/resources/js/prism-cpp.min.js:ro \
  -v $(pwd)/.github/prism-normalize-whitespace.min.js:/home/node/resources/js/prism-normalize-whitespace.min.js:ro \
  -v $(pwd)/.github/prism-toml.min.js:/home/node/resources/js/prism-toml.min.js:ro \
  -v $(pwd)/.github/prism-treeview.min.css:/home/node/resources/css/prism-treeview.min.css:ro \
  -v $(pwd)/.github/prism-treeview.min.js:/home/node/resources/js/prism-treeview.min.js:ro \
  -v $(pwd)/.github/tex-mml-chtml.js:/home/node/resources/js/tex-mml-chtml.js:ro \
  -e "PDF_OUTPUT_NAME=mjqm-documentation.pdf" \
  ghcr.io/kernoeb/docker-docsify-pdf:latest
