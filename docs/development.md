# Development

## Repository Structure

```text
.
|-- docs/
|-- shaders/
|-- src/
|-- tests/
|-- CMakeLists.txt
|-- Dockerfile
|-- README.md
`-- mkdocs.yml
```

## Tests

The project includes unit tests for the math helpers and particle-system logic.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure -C Release
```

## Documentation Stack

The docs site is built with MkDocs and deployed through GitHub Pages.

### Local preview

```bash
python3 -m pip install -r requirements-docs.txt
python3 -m mkdocs serve
```

### Production build

```bash
python3 -m mkdocs build --strict
```

The generated static site is written to `site/`.

## Release Notes For Docs Maintainers

- Site configuration lives in `mkdocs.yml`
- Content lives in `docs/`
- Visual customization is handled by `docs/stylesheets/terminal.css`
- The GitHub Actions Pages workflow installs MkDocs and publishes the `site/` directory
